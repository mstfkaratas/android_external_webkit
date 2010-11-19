/*
 * Copyright 2010, The Android Open Source Project
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "TexturesGenerator.h"

#if USE(ACCELERATED_COMPOSITING)

#include "GLUtils.h"
#include "TilesManager.h"

#ifdef DEBUG

#include <cutils/log.h>
#include <wtf/CurrentTime.h>
#include <wtf/text/CString.h>

#undef XLOG
#define XLOG(...) android_printLog(ANDROID_LOG_DEBUG, "TexturesGenerator", __VA_ARGS__)

#else

#undef XLOG
#define XLOG(...)

#endif // DEBUG

namespace WebCore {

void TexturesGenerator::schedulePaintForTileSet(TileSet* set)
{
    android::Mutex::Autolock lock(mRequestedPixmapsLock);
    for (unsigned int i = 0; i < mRequestedPixmaps.size(); i++) {
        TileSet** s = &mRequestedPixmaps[i];
        // A similar set is already in the queue. The newer set may have additional
        // dirty tiles so delete the existing set and replace it with the new one.
        if (*s && **s == *set) {
            TileSet* oldSet = *s;
            *s = set;
            delete oldSet;
            return;
        }
    }

    XLOG("%x schedulePaintForTileSet (%x) %d, %d, %d, %d", this, set,
        set->firstTileX(), set->firstTileY(), set->nbRows(), set->nbCols());
    mRequestedPixmaps.append(set);
    m_newRequestLock.lock();
    m_newRequestCond.signal();
    m_newRequestLock.unlock();
}

void TexturesGenerator::removeSetsWithPage(TiledPage* page)
{
    mRequestedPixmapsLock.lock();
    typedef Vector<TileSet*>::const_iterator iterator;
    iterator end = mRequestedPixmaps.end();
    for (iterator it = mRequestedPixmaps.begin(); it != end; ++it) {
        TileSet* set = static_cast<TileSet*>(*it);
        if (set->page() == page)
            delete *it;
    }
    TileSet* set = m_currentSet;
    if (set && set->page() != page)
        set = 0;
    if (set)
        m_waitForCompletion = true;
    mRequestedPixmapsLock.unlock();

    if (!set)
        return;

    // At this point, it means that we are currently painting a set that
    // we want to be removed -- we should wait until it is painted, so that
    // when we return our caller can be sure that there is no more TileSet
    // in the queue for that TiledPage and can safely deallocate the BaseTiles.
    mRequestedPixmapsLock.lock();
    mRequestedPixmapsCond.wait(mRequestedPixmapsLock);
    m_waitForCompletion = false;
    mRequestedPixmapsLock.unlock();
}

status_t TexturesGenerator::readyToRun()
{
    TilesManager::instance()->enableTextures();
    XLOG("Textures enabled (context acquired...)");
    TilesManager::instance()->paintTexturesDefault();
    XLOG("Textures painted");
    TilesManager::instance()->markGeneratorAsReady();
    XLOG("Thread ready to run");
    return NO_ERROR;
}

bool TexturesGenerator::threadLoop()
{
    mRequestedPixmapsLock.lock();

    if (!mRequestedPixmaps.size()) {
        XLOG("threadLoop, waiting for signal");
        m_newRequestLock.lock();
        mRequestedPixmapsLock.unlock();
        m_newRequestCond.wait(m_newRequestLock);
        m_newRequestLock.unlock();
        XLOG("threadLoop, got signal");
    } else {
        XLOG("threadLoop going as we already have something in the queue");
        mRequestedPixmapsLock.unlock();
    }

    m_currentSet = 0;
    bool stop = false;
    while (!stop) {
        mRequestedPixmapsLock.lock();
        if (mRequestedPixmaps.size()) {
            m_currentSet = mRequestedPixmaps.first();
            mRequestedPixmaps.remove(0);
        }
        mRequestedPixmapsLock.unlock();

        if (m_currentSet)
            m_currentSet->paint();

        mRequestedPixmapsLock.lock();
        if (m_currentSet) {
            delete m_currentSet;
            m_currentSet = 0;
            mRequestedPixmapsCond.signal();
        }
        if (!mRequestedPixmaps.size())
            stop = true;
        if (m_waitForCompletion)
            mRequestedPixmapsCond.signal();
        mRequestedPixmapsLock.unlock();
    }

    return true;
}

} // namespace WebCore

#endif // USE(ACCELERATED_COMPOSITING)
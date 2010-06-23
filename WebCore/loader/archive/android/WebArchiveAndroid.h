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

#ifndef WEBARCHIVEANDROID_H
#define WEBARCHIVEANDROID_H

#include "Archive.h"
#include "DocumentLoader.h"
#include "Frame.h"
#include <libxml/xmlwriter.h>

namespace WebCore {

class WebArchiveAndroid : public Archive {
public:
    static PassRefPtr<WebArchiveAndroid> create(PassRefPtr<ArchiveResource> mainResource,
        Vector<PassRefPtr<ArchiveResource> >& subresources,
        Vector<PassRefPtr<Archive> >& subframeArchives);

    static PassRefPtr<WebArchiveAndroid> create(Frame* frame);
    static PassRefPtr<WebArchiveAndroid> create(SharedBuffer* buffer);

    bool saveWebArchive(xmlTextWriterPtr writer);

private:
    WebArchiveAndroid(PassRefPtr<ArchiveResource> mainResource,
        Vector<PassRefPtr<ArchiveResource> >& subresources,
        Vector<PassRefPtr<Archive> >& subframeArchives);
};

}

#endif // WEBARCHIVEANDROID_H

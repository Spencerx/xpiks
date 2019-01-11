/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "generatethumbnailstemplate.h"

#include <QtDebug>

#include "Artworks/artworkssnapshot.h"
#include "Common/logging.h"
#include "QMLExtensions/imagecachingservice.h"
#include "QMLExtensions/videocachingservice.h"

namespace Commands {
    GenerateThumbnailsTemplate::GenerateThumbnailsTemplate(QMLExtensions::ImageCachingService &imageCachingService,
                                                         QMLExtensions::VideoCachingService &videoCachingService):
        m_ImageCachingService(imageCachingService),
        m_VideoCachingService(videoCachingService)
    {
    }

    void GenerateThumbnailsTemplate::execute(const Artworks::ArtworksSnapshot &snapshot) {
        LOG_DEBUG << "#";
        m_ImageCachingService.generatePreviews(snapshot);
        m_VideoCachingService.generateThumbnails(snapshot);
    }
}

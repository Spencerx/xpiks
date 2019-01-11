/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef GENERATETHUMBNAILSCOMMAND_H
#define GENERATETHUMBNAILSCOMMAND_H

#include "Commands/Base/icommandtemplate.h"

namespace QMLExtensions {
    class ImageCachingService;
    class VideoCachingService;
}

namespace Artworks {
    class ArtworksSnapshot;
}

namespace Commands {
    class GenerateThumbnailsTemplate: public ICommandTemplate<Artworks::ArtworksSnapshot>
    {
    public:
        GenerateThumbnailsTemplate(QMLExtensions::ImageCachingService &imageCachingService,
                                   QMLExtensions::VideoCachingService &videoCachingService);

        // ICommand interface
    public:
        virtual void execute(const Artworks::ArtworksSnapshot &snapshot) override;

    private:
        QMLExtensions::ImageCachingService &m_ImageCachingService;
        QMLExtensions::VideoCachingService &m_VideoCachingService;
    };
}

#endif // GENERATETHUMBNAILSCOMMAND_H

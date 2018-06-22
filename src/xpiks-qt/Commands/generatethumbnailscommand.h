/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef GENERATETHUMBNAILSCOMMAND_H
#define GENERATETHUMBNAILSCOMMAND_H

#include "icommand.h"
#include <memory>

namespace QMLExtensions {
    class ImageCachingService;
    class VideoCachingService;
}

namespace Artworks {
    class ArtworksSnapshot;
}

namespace Commands {
    class GenerateThumbnailsCommand: public ICommand
    {
    public:
        GenerateThumbnailsCommand(const std::shared_ptr<Artworks::ArtworksSnapshot> &snapshot,
                                  QMLExtensions::ImageCachingService &imageCachingService,
                                  QMLExtensions::VideoCachingService &videoCachingService);

        // ICommand interface
    public:
        virtual void execute() override;

    private:
        std::shared_ptr<Artworks::ArtworksSnapshot> m_Snapshot;
        QMLExtensions::ImageCachingService &m_ImageCachingService;
        QMLExtensions::VideoCachingService &m_VideoCachingService;
    };
}

#endif // GENERATETHUMBNAILSCOMMAND_H

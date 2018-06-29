/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef UPDATEVIDEOTHUMBNAILTEMPLATE_H
#define UPDATEVIDEOTHUMBNAILTEMPLATE_H

#include <Commands/Base/icommandtemplate.h>

namespace Artworks {
    class ArtworksSnapshot;
}

namespace QMLExtensions {
    class VideoCachingService;
}

namespace Services {
    class ArtworksUpdateHub;
}

namespace Commands {
    class UpdateVideoThumbnailTemplate: public ICommandTemplate<Artworks::ArtworksSnapshot>
    {
    public:
        UpdateVideoThumbnailTemplate(QMLExtensions::VideoCachingService &videoCachingService,
                                     Services::ArtworksUpdateHub &artworksUpdateHub);

    public:
        virtual void execute(const Artworks::ArtworksSnapshot &snapshot) = 0;
        virtual void undo(const Artworks::ArtworksSnapshot& snapshot) { execute(snapshot); }

    private:
        QMLExtensions::VideoCachingService &m_VideoCachingService;
        Services::ArtworksUpdateHub &m_ArtworksUpdateHub;
    };
}

#endif // UPDATEVIDEOTHUMBNAILTEMPLATE_H

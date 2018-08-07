/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef IARTWORKSUPDATER_H
#define IARTWORKSUPDATER_H

namespace Artworks {
    class ArtworksSnapshot;
}

namespace Services {
    enum UpdateMode {
        FastUpdate,
        FullUpdate
    };

    class IArtworksUpdater {
    public:
        virtual ~IArtworksUpdater() {}
        virtual void updateArtworks(Artworks::ArtworksSnapshot const &artworks, UpdateMode updateMode=FastUpdate) = 0;
    };
}

#endif // IARTWORKSUPDATER_H

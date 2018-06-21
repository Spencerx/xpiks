/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef ICURRENTARTWORKSOURCE_H
#define ICURRENTARTWORKSOURCE_H

#include "artworkmetadata.h"

namespace Artworks {
    class ICurrentArtworkSource {
    public:
        virtual ~ICurrentArtworkSource() {}
        virtual ArtworkMetadata *getCurrentArtwork() = 0;
    };
}

#endif // ICURRENTARTWORKSOURCE_H

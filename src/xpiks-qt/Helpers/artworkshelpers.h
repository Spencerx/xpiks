/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef ARTWORKSHELPERS_H
#define ARTWORKSHELPERS_H

#include <QVector>
#include <vector>
#include <memory>

namespace Artworks {
    class ArtworkMetadata;
    class ArtworksSnapshot;
}

namespace Helpers {
    void splitImagesVideo(const QVector<Artworks::ArtworkMetadata *> &artworks,
                          QVector<Artworks::ArtworkMetadata *> &imageArtworks,
                          QVector<Artworks::ArtworkMetadata *> &videoArtworks);
    void splitImagesVideo(const Artworks::ArtworksSnapshot &rawSnapshot,
                          Artworks::ArtworksSnapshot &imagesRawSnapshot,
                          Artworks::ArtworksSnapshot &videoRawSnapshot);
    int retrieveImagesCount(const Artworks::ArtworksSnapshot &rawSnapshot);
    int retrieveVideosCount(const Artworks::ArtworksSnapshot &rawSnapshot);
    int findAndAttachVectors(const Artworks::ArtworksSnapshot &snapshot, QVector<int> &modifiedIndices);
}

#endif // ARTWORKSHELPERS_H

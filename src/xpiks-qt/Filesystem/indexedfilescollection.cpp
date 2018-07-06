/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "indexedfilescollection.h"
#include <Artworks/artworkssnapshot.h>
#include <Artworks/imageartwork.h>

namespace Filesystem {
    IndexedFilesCollection::IndexedFilesCollection(const Artworks::ArtworksSnapshot &snapshot,
                                                   std::vector<int> &indices)
    {
        Q_ASSERT(snapshot.size() == indices.size());
        const size_t size = indices.size();
        m_Files.reserve(size);
        for (size_t i = 0; i < size; i++) {
            Artworks::ArtworkMetadata *artwork = snapshot.get(i);
            Artworks::ImageArtwork *image = dynamic_cast<Artworks::ImageArtwork*>(artwork);
            const bool isImage = image != nullptr;
            m_Files.emplace_back(artwork->getFilepath(),
                                 isImage ? ArtworkFileType::Image : ArtworkFileType::Video);
            if (isImage && image->hasVectorAttached()) {
                m_Files.emplace_back(image->getAttachedVectorPath(),
                                     ArtworkFileType::Vector);
            }
        }
    }
}

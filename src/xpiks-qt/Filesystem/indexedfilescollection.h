/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INDEXEDFILESCOLLECTION_H
#define INDEXEDFILESCOLLECTION_H

#include "ifilescollection.h"

namespace MetadataIO {
    class ArtworksSnapshot;
}

namespace Filesystem {
    class IndexedFilesCollection: public IFilesCollection
    {
    public:
        IndexedFilesCollection(const Artworks::ArtworksSnapshot &snapshot, std::vector<int> &indices);

        // IFilesCollection interface
    public:
        virtual const std::vector<ArtworkFile> &getFiles() override { return m_Files; }

    private:
        std::vector<ArtworkFile> m_Files;
    };
}

#endif // INDEXEDFILESCOLLECTION_H

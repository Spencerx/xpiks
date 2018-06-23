/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "readmetadatatemplate.h"
#include "../MetadataIO/metadataioservice.h"
#include "../MetadataIO/metadataiocoordinator.h"
#include "../Artworks/artworkssnapshot.h"

namespace Commands {
    ReadMetadataTemplate::ReadMetadataTemplate(MetadataIO::MetadataIOService &metadataIOService,
                                               MetadataIO::MetadataIOCoordinator &metadataIOCoordinator):
        m_MetadataIOService(metadataIOService),
        m_MetadataIOCoordinator(metadataIOCoordinator)
    {
    }

    void ReadMetadataTemplate::execute(Artworks::ArtworksSnapshot &snapshot) {
        LOG_DEBUG << "#";
        quint32 batchID = m_MetadataIOService.readArtworks(snapshot);
        int importID = m_MetadataIOCoordinator.readMetadataExifTool(snapshot, batchID);
        Q_UNUSED(importID);
    }
}

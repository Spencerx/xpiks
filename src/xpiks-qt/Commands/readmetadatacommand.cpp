/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "readmetadatacommand.h"
#include "../MetadataIO/metadataioservice.h"
#include "../MetadataIO/metadataiocoordinator.h"
#include "../Artworks/artworkssnapshot.h"

namespace Commands {
    ReadMetadataCommand::ReadMetadataCommand(const std::shared_ptr<Artworks::ArtworksSnapshot> &snapshot,
                                             MetadataIO::MetadataIOService &metadataIOService,
                                             MetadataIO::MetadataIOCoordinator &metadataIOCoordinator):
        m_Snapshot(snapshot),
        m_MetadataIOService(metadataIOService),
        m_MetadataIOCoordinator(metadataIOCoordinator)
    {
    }

    void ReadMetadataCommand::execute() {
        quint32 batchID = m_MetadataIOService.readArtworks(*m_Snapshot.get());
        int importID = m_MetadataIOCoordinator.readMetadataExifTool(*m_Snapshot.get(), batchID);
        Q_UNUSED(importID);
    }
}

/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "savebackupscommand.h"
#include "../MetadataIO/metadataioservice.h"

namespace Commands {
    SaveBackupsCommand::SaveBackupsCommand(const std::shared_ptr<Artworks::ArtworksSnapshot> &snapshot,
                                           MetadataIO::MetadataIOService &metadataIOService):
        m_Snapshot(snapshot),
        m_MetadataIOService(metadataIOService)
    {
    }

    void SaveBackupsCommand::execute() {
        m_MetadataIOService.writeArtworks(m_Snapshot->getWeakSnapshot());
    }
}

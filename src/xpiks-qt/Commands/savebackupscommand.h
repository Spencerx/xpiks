/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SAVEBACKUPSCOMMAND_H
#define SAVEBACKUPSCOMMAND_H

#include "icommand.h"
#include <memory>

namespace MetadataIO {
    class MetadataIOService;
}

namespace Artworks {
    class ArtworksSnapshot;
}

namespace Commands {
    class SaveBackupsCommand : public ICommand
    {
    public:
        SaveBackupsCommand(const std::shared_ptr<Artworks::ArtworksSnapshot> &snapshot,
                           MetadataIO::MetadataIOService &metadataIOService);

        // ICommand interface
    public:
        virtual void execute() override;

    private:
        std::shared_ptr<Artworks::ArtworksSnapshot> m_Snapshot;
        MetadataIO::MetadataIOService &m_MetadataIOService;
    };
}

#endif // SAVEBACKUPSCOMMAND_H

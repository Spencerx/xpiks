/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef READMETADATACOMMAND_H
#define READMETADATACOMMAND_H

#include <memory>
#include "icommand.h"

namespace MetadataIO {
    class MetadataIOService;
    class MetadataIOCoordinator;
}

namespace Artworks {
    class ArtworksSnapshot;
}

namespace Commands {
    class ReadMetadataCommand: public ICommand
    {
    public:
        ReadMetadataCommand(const std::shared_ptr<Artworks::ArtworksSnapshot> &snapshot,
                            MetadataIO::MetadataIOService &metadataIOService,
                            MetadataIO::MetadataIOCoordinator &metadataIOCoordinator);

        // ICommand interface
    public:
        virtual void execute() override;

    private:
        std::shared_ptr<Artworks::ArtworksSnapshot> m_Snapshot;
        MetadataIO::MetadataIOService &m_MetadataIOService;
        MetadataIO::MetadataIOCoordinator &m_MetadataIOCoordinator;
    };
}

#endif // READMETADATACOMMAND_H

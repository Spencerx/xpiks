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
#include "iartworkscommandtemplate.h"

namespace MetadataIO {
    class MetadataIOService;
    class MetadataIOCoordinator;
}

namespace Commands {
    class ReadMetadataTemplate: public IArtworksCommandTemplate
    {
    public:
        ReadMetadataTemplate(MetadataIO::MetadataIOService &metadataIOService,
                             MetadataIO::MetadataIOCoordinator &metadataIOCoordinator);

        // ICommand interface
    public:
        virtual void execute(Artworks::ArtworksSnapshot &snapshot) override;

    private:
        MetadataIO::MetadataIOService &m_MetadataIOService;
        MetadataIO::MetadataIOCoordinator &m_MetadataIOCoordinator;
    };
}

#endif // READMETADATACOMMAND_H

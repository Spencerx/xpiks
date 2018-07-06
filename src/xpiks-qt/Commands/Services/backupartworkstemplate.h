/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef BACKUPARTWORKSTEMPLATE_H
#define BACKUPARTWORKSTEMPLATE_H

#include <Commands/Base/icommandtemplate.h>

namespace MetadataIO {
    class MetadataIOService;
}

namespace Artworks {
    class ArtworksSnapshot;
}

namespace Commands {
    class BackupArtworksTemplate: public ICommandTemplate<Artworks::ArtworksSnapshot>
    {
    public:
        BackupArtworksTemplate(MetadataIO::MetadataIOService &metadataIOService);

        // IArtworksCommandTemplate interface
    public:
        virtual void execute(const Artworks::ArtworksSnapshot &snapshot) override;
        virtual void undo(const Artworks::ArtworksSnapshot &snapshot) override { execute(snapshot); }

    private:
        MetadataIO::MetadataIOService &m_MetadataIOService;
    };
}

#endif // BACKUPARTWORKSTEMPLATE_H

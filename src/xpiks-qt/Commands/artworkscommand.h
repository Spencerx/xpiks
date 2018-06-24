/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef ARTWORKSSNAPSHOTCOMMAND_H
#define ARTWORKSSNAPSHOTCOMMAND_H

#include "icommand.h"
#include "iartworkscommandtemplate.h"
#include "../Artworks/artworkssnapshot.h"
#include <memory>

namespace Artworks {
    class ArtworkMetadata;
}

namespace Commands {
    class ArtworksCommand: ICommand {
    public:
        ArtworksCommand(Artworks::ArtworksSnapshot &snapshot,
                        std::shared_ptr<IArtworksCommandTemplate> &artworkTemplate):
            m_Snapshot(std::move(snapshot)),
            m_ArtworkTemplate(std::move(artworkTemplate))
        {
            Q_ASSERT(m_ArtworkTemplate);
        }

        ArtworksCommand(Artworks::ArtworkMetadata *artwork,
                        std::shared_ptr<IArtworksCommandTemplate> &artworkTemplate):
            m_Snapshot({artwork}),
            m_ArtworkTemplate(std::move(artworkTemplate))
        {
            Q_ASSERT(m_ArtworkTemplate);
        }

        // ICommand interface
    public:
        virtual void execute() override { m_ArtworkTemplate->execute(m_Snapshot; }
        virtual void undo() override { return m_ArtworkTemplate->undo(m_Snapshot); }

    private:
        Artworks::ArtworksSnapshot m_Snapshot;
        std::shared_ptr<IArtworksCommandTemplate> m_ArtworkTemplate;
    };
}

#endif // ARTWORKSSNAPSHOTCOMMAND_H

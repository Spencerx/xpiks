/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef ARTWORKSUPDATETEMPLATE_H
#define ARTWORKSUPDATETEMPLATE_H

#include <QVector>

#include "Commands/Base/icommandtemplate.h"

namespace Models {
    class ArtworksListModel;
}

namespace Artworks {
    class ArtworksSnapshot;
}

namespace Services {
    class IArtworksUpdater;
}

namespace Commands {
    class ArtworksUpdateTemplate: public ICommandTemplate<Artworks::ArtworksSnapshot>
    {
    public:
        ArtworksUpdateTemplate(Models::ArtworksListModel &artworksListModel,
                               const QVector<int> &roles = QVector<int>());

        // IArtworksCommandTemplate interface
    public:
        virtual void execute(const Artworks::ArtworksSnapshot &snapshot) override;
        virtual void undo(const Artworks::ArtworksSnapshot &snapshot) override { execute(snapshot); }

    private:
        Models::ArtworksListModel &m_ArtworksListModel;
        QVector<int> m_Roles;
    };

    class ArtworksSnapshotUpdateTemplate: public ICommandTemplate<Artworks::ArtworksSnapshot>
    {
    public:
        ArtworksSnapshotUpdateTemplate(Services::IArtworksUpdater &updater);

        // IArtworksCommandTemplate interface
    public:
        virtual void execute(const Artworks::ArtworksSnapshot &snapshot) override;
        virtual void undo(const Artworks::ArtworksSnapshot &snapshot) override { execute(snapshot); }

    private:
        Services::IArtworksUpdater &m_Updater;
    };
}

#endif // ARTWORKSUPDATETEMPLATE_H

/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "artworksupdatetemplate.h"
#include <Helpers/cpphelpers.h>
#include <Artworks/artworkmetadata.h>
#include <Artworks/artworkssnapshot.h>
#include <Models/Artworks/artworkslistmodel.h>
#include <Services/iartworksupdater.h>

namespace Commands {
    ArtworksUpdateTemplate::ArtworksUpdateTemplate(Models::ArtworksListModel &artworksListModel,
                                                   const QVector<int> &roles):
        m_ArtworksListModel(artworksListModel),
        m_Roles(roles)
    {
        if (m_Roles.isEmpty()) {
            m_Roles = artworksListModel.getStandardUpdateRoles();
        }
    }

    void ArtworksUpdateTemplate::execute(const Artworks::ArtworksSnapshot &snapshot) {
        LOG_DEBUG << "#";
        auto indices = Helpers::map<Artworks::ArtworksSnapshot::ItemType, int>(
                    snapshot.getRawData(),
                    [](const Artworks::ArtworksSnapshot::ItemType &artwork) {
            return artwork->getLastKnownIndex();
        });

        m_ArtworksListModel.updateItems(Helpers::IndicesRanges(indices), m_Roles);
    }

    ArtworksSnapshotUpdateTemplate::ArtworksSnapshotUpdateTemplate(Services::IArtworksUpdater &updater):
        m_Updater(updater)
    {
    }

    void ArtworksSnapshotUpdateTemplate::execute(const Artworks::ArtworksSnapshot &snapshot) {
        LOG_DEBUG << "#";
        m_Updater.updateArtworks(snapshot);
    }
}

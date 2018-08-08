/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "removefilescommandbase.h"
#include <Models/Artworks/artworkslistmodel.h>
#include <Models/Artworks/artworksrepository.h>

namespace Commands {
    RemoveFilesCommandBase::RemoveFilesCommandBase(Models::ArtworksListModel &artworksList,
                                                   Models::ArtworksRepository &artworksRepository):
        m_ArtworksList(artworksList),
        m_ArtworksRepository(artworksRepository)
    {
    }

    void RemoveFilesCommandBase::undo() {
        LOG_DEBUG << "#";
        if (m_RemoveResult.m_UnselectAll && m_ArtworksRepository.allAreSelected()) {
            m_ArtworksRepository.unselectAllDirectories();
        }

        restoreFiles();

        m_ArtworksRepository.restoreDirectoriesSelection(m_RemoveResult.m_SelectedDirectoryIds);
        // TODO: notify warnings model
    }

    void RemoveFilesCommandBase::restoreFiles() {
        m_ArtworksList.restoreRemoved();
    }
}

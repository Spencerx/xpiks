/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "removefilescommand.h"
#include "../Models/artworkslistmodel.h"
#include "../Models/artworksrepository.h"

namespace Commands {
    RemoveFilesCommand::RemoveFilesCommand(Models::ArtworksListModel &artworksList,
                                           Models::ArtworksRepository &artworksRepository):
        m_ArtworksList(artworksList),
        m_ArtworksRepository(artworksRepository)
    {
    }

    std::shared_ptr<CommandResult> RemoveFilesCommand::execute(int commandID) {
        m_CommandID = commandID;
        m_RemoveResult = removeFiles();
        return ICommand::execute();
    }

    void RemoveFilesCommand::undo() {
        LOG_DEBUG << "#";
        if (m_RemoveResult.m_UnselectAll && m_ArtworksRepository.allAreSelected()) {
            m_ArtworksRepository.unselectAllDirectories();
        }

        m_ArtworksList.restoreRemoved();

        m_ArtworksRepository.restoreDirectoriesSelection(m_RemoveResult.m_SelectedDirectoryIds);
    }
}

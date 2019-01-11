/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "removefilescommandbase.h"

#include <QtDebug>

#include "Common/logging.h"
#include "Commands/Base/icommand.h"
#include "Models/Artworks/artworkslistmodel.h"
#include "Models/Artworks/artworkslistoperations.h"
#include "Models/Artworks/artworksrepository.h"

namespace Commands {
    RemoveFilesCommandBase::RemoveFilesCommandBase(Models::ArtworksListModel &artworksList,
                                                   Models::ArtworksRepository &artworksRepository,
                                                   std::shared_ptr<ICommand> const &saveSessionCommand):
        m_ArtworksList(artworksList),
        m_ArtworksRepository(artworksRepository),
        m_RemoveResult(),
        m_SaveSessionCommand(saveSessionCommand)
    {
    }

    void RemoveFilesCommandBase::execute() {
        LOG_DEBUG << "#";
        m_RemoveResult = removeFiles();

        if (m_RemoveResult.m_RemovedCount > 0) {
            if (m_SaveSessionCommand) {
                m_SaveSessionCommand->execute();
            }
        }
    }

    void RemoveFilesCommandBase::undo() {
        LOG_DEBUG << "#";
        if (m_RemoveResult.m_UnselectAll && m_ArtworksRepository.allAreSelected()) {
            m_ArtworksRepository.unselectAllDirectories();
        }

        restoreFiles();

        m_ArtworksRepository.restoreDirectoriesSelection(m_RemoveResult.m_SelectedDirectoryIds);

        if (m_SaveSessionCommand) {
            m_SaveSessionCommand->execute();
        }
    }

    void RemoveFilesCommandBase::restoreFiles() {
        m_ArtworksList.restoreRemoved(m_RemoveResult.m_RemovedRanges);
    }
}

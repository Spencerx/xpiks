/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "removeselectedfilescommand.h"
#include <Artworks/iselectedindicessource.h>
#include <Models/Artworks/artworkslistmodel.h>

namespace Commands {
    RemoveSelectedFilesCommand::RemoveSelectedFilesCommand(Artworks::ISelectedIndicesSource &indicesSource,
                                                           Models::ArtworksListModel &artworksList,
                                                           Models::ArtworksRepository &artworksRepository,
                                                           std::shared_ptr<ICommand> const &saveSessionCommand):
        RemoveFilesCommandBase(artworksList, artworksRepository),
        m_IndicesSource(indicesSource),
        m_SaveSessionCommand(saveSessionCommand)
    {
    }

    void RemoveSelectedFilesCommand::execute() {
        LOG_DEBUG << "#";
        auto indices = m_IndicesSource.getSelectedIndices();
        m_RemoveResult = m_ArtworksList.removeFiles(Helpers::IndicesRanges(indices));
        if (m_RemoveResult.m_RemovedCount > 0) {
            if (m_SaveSessionCommand) {
                m_SaveSessionCommand->execute();
            }
        }
    }

    void RemoveSelectedFilesCommand::undo() {
        LOG_DEBUG << "#";

        RemoveFilesCommandBase::undo();

        if (m_SaveSessionCommand) {
            m_SaveSessionCommand->execute();
        }
    }
}

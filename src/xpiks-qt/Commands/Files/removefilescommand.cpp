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

#include <QtDebug>

#include "Commands/Base/icommand.h"
#include "Common/logging.h"
#include "Models/Artworks/artworkslistmodel.h"

namespace Commands {
    RemoveFilesCommand::RemoveFilesCommand(const Helpers::IndicesRanges &ranges,
                                           Models::ArtworksListModel &artworksList,
                                           Models::ArtworksRepository &artworksRepository,
                                           std::shared_ptr<ICommand> const &saveSessionCommand):
        RemoveFilesCommandBase(artworksList, artworksRepository),
        m_Ranges(ranges),
        m_SaveSessionCommand(saveSessionCommand)
    {
    }

    void RemoveFilesCommand::execute() {
        LOG_DEBUG << "#";
        RemoveFilesCommandBase::execute();

        if (m_RemoveResult.m_RemovedCount > 0) {
            if (m_SaveSessionCommand) {
                m_SaveSessionCommand->execute();
            }
        }
    }

    void RemoveFilesCommand::undo() {
        LOG_DEBUG << "#";

        RemoveFilesCommandBase::undo();

        if (m_SaveSessionCommand) {
            m_SaveSessionCommand->execute();
        }
    }

    Models::ArtworksRemoveResult RemoveFilesCommand::removeFiles() {
        return m_ArtworksList.removeFiles(m_Ranges);
    }
}

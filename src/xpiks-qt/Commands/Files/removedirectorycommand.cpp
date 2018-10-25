/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "removedirectorycommand.h"

#include <initializer_list>

#include <QSet>
#include <QtDebug>
#include <QtGlobal>

#include "Artworks/artworkssnapshot.h"
#include "Commands/Base/icommandtemplate.h"
#include "Commands/Files/removefilescommandbase.h"
#include "Common/flags.h"
#include "Common/logging.h"
#include "Filesystem/directoriescollection.h"
#include "Models/Artworks/artworkslistmodel.h"
#include "Models/Artworks/artworksrepository.h"

namespace Commands {
    RemoveDirectoryCommand::RemoveDirectoryCommand(int originalIndex,
                                                   Models::ArtworksListModel &artworksList,
                                                   Models::ArtworksRepository &artworksRepository,
                                                   Models::SettingsModel &settingsModel,
                                                   ArtworksCommandTemplate const &addedArtworksTemplate):
        RemoveFilesCommandBase(artworksList, artworksRepository),
        m_DirectoryIndex(originalIndex),
        m_SettingsModel(settingsModel),
        m_AddedArtworksTemplate(addedArtworksTemplate),
        m_IsFullDirectory(false)
    {
    }

    Models::ArtworksRemoveResult RemoveDirectoryCommand::removeFiles() {
        LOG_DEBUG << "#";
        const qint64 directoryID = m_ArtworksRepository.getDirectoryID(m_DirectoryIndex);
        m_DirectoryPath = m_ArtworksRepository.getDirectoryPath(m_DirectoryIndex);
        Models::ArtworksRemoveResult result = m_ArtworksList.removeFilesFromDirectory(m_DirectoryIndex);
        m_IsFullDirectory = result.m_FullDirectoryIds.contains(directoryID);
        return result;
    }

    void RemoveDirectoryCommand::restoreFiles() {
        LOG_DEBUG << "#";
        RemoveFilesCommandBase::restoreFiles();

        if (m_IsFullDirectory) {
#ifndef CORE_TESTS
            auto filesCollection = std::make_shared<Filesystem::DirectoriesCollection>(
                                       std::initializer_list<QString>{m_DirectoryPath});
#else
            auto filesCollection = m_FakeFiles;
#endif
            Common::AddFilesFlags flags = Common::AddFilesFlags::FlagIsFullDirectory;
            auto addResult = m_ArtworksList.addFiles(filesCollection, flags);

            if (m_AddedArtworksTemplate) {
                m_AddedArtworksTemplate->execute(addResult.m_Snapshot);
            }

            emit artworksAdded((int)addResult.m_Snapshot.size(), (int)addResult.m_AttachedVectorsCount);
        }
    }
}

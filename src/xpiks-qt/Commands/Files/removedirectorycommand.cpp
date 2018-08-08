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
#include <Common/logging.h>
#include <Models/Artworks/artworkslistmodel.h>
#include <Models/Artworks/artworksrepository.h>
#include <Filesystem/directoriescollection.h>

namespace Commands {
    RemoveDirectoryCommand::RemoveDirectoryCommand(int directoryID,
                                                   Models::ArtworksListModel &artworksList,
                                                   Models::ArtworksRepository &artworksRepository,
                                                   Models::SettingsModel &settingsModel,
                                                   ArtworksCommandTemplate const &addedArtworksTemplate):
        RemoveFilesCommandBase(artworksList, artworksRepository),
        m_DirectoryID(directoryID),
        m_SettingsModel(settingsModel),
        m_AddedArtworksTemplate(addedArtworksTemplate),
        m_IsFullDirectory(false)
    {
    }

    void RemoveDirectoryCommand::execute() {
        LOG_DEBUG << "#";
        bool foundDirectory = m_ArtworksRepository.tryGetDirectoryPath(m_DirectoryID, m_DirectoryPath);
        Q_ASSERT(foundDirectory);
        m_RemoveResult = m_ArtworksList.removeFilesFromDirectory(m_DirectoryID);
        m_IsFullDirectory = m_RemoveResult.m_FullDirectoryIds.contains(m_DirectoryID);
    }

    void RemoveDirectoryCommand::restoreFiles() {
        LOG_DEBUG << "#";
        RemoveFilesCommandBase::restoreFiles();

        if (m_IsFullDirectory) {
#ifndef CORE_TESTS
            auto filesCollection = std::make_shared<Filesystem::DirectoriesCollection>(std::initializer_list<QString>{m_DirectoryPath});
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

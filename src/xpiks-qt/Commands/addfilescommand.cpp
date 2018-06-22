/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "addfilescommand.h"
#include "../Models/artworkslistmodel.h"
#include "../Common/logging.h"
#include "../Helpers/indicesranges.h"

namespace Commands {
    std::shared_ptr<Commands::CommandResult> AddFilesCommand::execute(int commandID) {
        LOG_DEBUG << "#";
        m_CommandID = commandID;

        m_OriginalCount = m_ArtworksListModel.getArtworksCount();
        m_AddedCount = addFiles();

        // clean resources if this will be stored in undo manager
        m_Files.reset();
        m_ClearLegacyBackupsCommand.reset();

        return IAppCommand::execute();
    }

    int AddFilesCommand::addFiles() {
        auto addResult = m_ArtworksListModel.addFiles(m_Files, m_Flags);

        m_RecentFileModel.add(addResult.m_Snapshot);
        saveSession();

        const bool autoImportEnabled = m_SettingsModel.getUseAutoImport() && m_SwitcherModel.getUseAutoImport();
        if (autoImportEnabled) {
            LOG_DEBUG << "Autoimport is ON. Proceeding...";
            m_MetadataIOCoordinator.continueReading(false);
        }

        emit artworksAdded(importID, addResult.m_Snapshot.size(), addResult.m_AttachedVectorsCount);

        m_ClearLegacyBackupsCommand->execute();

        return addResult.m_Snapshot.size();
    }

    void AddFilesCommand::saveSession() {
        if (!Common::HasFlag(m_Flags, Common::AddFilesFlags::FlagIsSessionRestore)) {
            m_SaveSessionCommand->execute();
        }
    }

    void AddFilesCommand::undo() {
        LOG_DEBUG << "#";
        m_ArtworksListModel.removeFiles(Helpers::IndicesRanges(m_OriginalCount, m_AddedCount));
        saveSession();
    }
}

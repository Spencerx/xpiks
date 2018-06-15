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
#include "../Models/artitemsmodel.h"
#include "../MetadataIO/metadataioservice.h"
#include "../MetadataIO/metadataiocoordinator.h"
#include "../UndoRedo/addartworksitem.h"
#include "../Common/logging.h"

namespace Commands {
    std::shared_ptr<Commands::CommandResult> AddFilesCommand::execute() {
        const int count = m_ArtItemsModel.getArtworksCount();
        auto addResult = m_ArtItemsModel.addFiles(m_Files, m_Flags);

        quint32 batchID = m_MetadataIOService.readArtworks(addResult.m_Snapshot);
        int importID = m_MetadataIOCoordinator.readMetadataExifTool(addResult.m_Snapshot, batchID);

        m_ImageCachingService.generatePreviews(addResult.m_Snapshot);
        m_VideoCachingService.generateThumbnails(addResult.m_Snapshot);

        m_RecentFileModel.add(addResult.m_Snapshot);

        if (!Common::HasFlag(m_Flags, Common::AddFilesFlags::FlagIsSessionRestore)) {
            m_SaveSessionCommand->execute();
        }

        m_UndoRedoManager.recordHistoryItem(std::make_unique<UndoRedo::IHistoryItem>(
                                                new UndoRedo::AddArtworksHistoryItem(
                                                    m_ArtItemsModel, count, addResult.m_Snapshot.size(),
                                                    m_SaveSessionCommand)));

        const bool autoImportEnabled = m_SettingsModel.getUseAutoImport() && m_SwitcherModel.getUseAutoImport();
        if (autoImportEnabled) {
            LOG_DEBUG << "Autoimport is ON. Proceeding...";
            m_MetadataIOCoordinator.continueReading(false);
        }

        emit artworksAdded(importID, addResult.m_Snapshot.size(), addResult.m_AttachedVectorsCount);

        if (m_ClearLegacyBackupsCommand) {
            m_ClearLegacyBackupsCommand->execute();
        }

        return ICommand::execute();
    }
}

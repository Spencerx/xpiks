/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "modifyartworkshistoryitem.h"
#include "../Models/artitemsmodel.h"
#include "../Models/artworkmetadata.h"
#include "../Commands/commandmanager.h"
#include "../Models/settingsmodel.h"
#include "../Common/defines.h"
#include "../MetadataIO/artworkssnapshot.h"

void UndoRedo::ModifyArtworksHistoryItem::undo(const Commands::ICommandManager *commandManagerInterface) {
    LOG_INFO << m_Indices.count() << "item(s) affected";

    Commands::CommandManager *commandManager = (Commands::CommandManager*)commandManagerInterface;
    auto *xpiks = commandManager->getDelegator();

    Models::ArtItemsModel *artItemsModel = commandManager->getArtItemsModel();
    int count = m_Indices.count();

    Artworks::WeakArtworksSnapshot itemsToSave;
    itemsToSave.reserve(count);

    for (int i = 0; i < count; ++i) {
        int index = m_Indices[i];
        Artworks::ArtworkMetadata *artwork = artItemsModel->getArtwork(index);
        if (artwork != NULL) {
            const ArtworkMetadataBackup &backup = m_ArtworksBackups.at(i);
            backup.restore(artwork);
            itemsToSave.push_back(artwork);
        }
    }

    xpiks->submitForSpellCheck(itemsToSave);
    xpiks->saveArtworksBackups(itemsToSave);
    artItemsModel->updateItemsAtIndices(m_Indices);
    artItemsModel->updateModifiedCount();
}



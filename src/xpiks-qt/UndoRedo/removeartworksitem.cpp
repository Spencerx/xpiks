/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QFileInfo>
#include "removeartworksitem.h"
#include "../Commands/commandmanager.h"
#include "../Common/defines.h"
#include "../Helpers/indiceshelper.h"
#include "../Models/artworksrepository.h"
#include "../Models/artitemsmodel.h"
#include "../Models/artworkmetadata.h"
#include "../Models/imageartwork.h"
#include "addartworksitem.h"
#include "../MetadataIO/metadataiocoordinator.h"

void UndoRedo::RemoveArtworksHistoryItem::undo(const Commands::ICommandManager *commandManagerInterface) {
    LOG_INFO << "#";

    Commands::CommandManager *commandManager = (Commands::CommandManager*)commandManagerInterface;

    QVector<QPair<int, int> > ranges;
    Helpers::indicesToRanges(m_RemovedArtworksIndices, ranges);

    Models::ArtItemsModel *artItemsModel = commandManager->getArtItemsModel();
    Models::ArtworksRepository *artworksRepository = commandManager->getArtworksRepository();

    MetadataIO::ArtworksSnapshot artworksToImport;
    artworksToImport.reserve(m_RemovedArtworksIndices.length());
    QStringList watchList;
    watchList.reserve(m_RemovedArtworksIndices.length());

    bool filesWereAccounted = artworksRepository->beginAccountingFiles(m_RemovedArtworksPathes);
    const bool willResetModel = m_RemovedArtworksPathes.length() > artItemsModel->getMinChangedItemsCountForReset();

    int usedCount = 0, attachedVectors = 0;
    int rangesCount = ranges.count();

    if (willResetModel) {
        artItemsModel->beginAccountingManyFiles();
    }

    if (m_UnselectAll && artworksRepository->allAreSelected()) {
        artworksRepository->unselectAllDirectories();
    }

    for (int i = 0; i < rangesCount; ++i) {
        int startRow = ranges[i].first;
        int endRow = ranges[i].second;

        if (!willResetModel) {
            artItemsModel->beginAccountingFiles(startRow, endRow);
        }

        Common::flag_t directoryFlags = 0;
        if (m_RemovedAsDirectory) { Common::SetFlag(directoryFlags, Common::DirectoryFlags::IsAddedAsDirectory); }

        int count = endRow - startRow + 1;
        for (int j = 0; j < count; ++j) {
            const QString &filepath = m_RemovedArtworksPathes[j + usedCount];
            qint64 directoryID = 0;
            if (artworksRepository->accountFile(filepath, directoryID, directoryFlags)) {
                Models::ArtworkMetadata *artwork = artItemsModel->createArtwork(filepath, directoryID);

                artItemsModel->insertArtwork(j + startRow, artwork);
                artworksToImport.append(artwork);
                watchList.append(filepath);

                const QString &vectorPath = m_RemovedAttachedVectors.at(j);
                if (!vectorPath.isEmpty()) {
                    Models::ImageArtwork *image = dynamic_cast<Models::ImageArtwork*>(artwork);
                    if (image != NULL) {
                        image->attachVector(vectorPath);
                        attachedVectors++;
                    } else {
                        LOG_WARNING << "Vector was attached not to an image!";
                    }
                }
            }
        }

        artworksRepository->restoreDirectoriesSelection(m_RemovedSelectedDirectoryIds);

        if (!willResetModel) {
            artItemsModel->endAccountingFiles();
        }

        usedCount += (endRow - startRow + 1);
    }

    if (willResetModel) {
        artItemsModel->endAccountingManyFiles();
    }

    artworksRepository->endAccountingFiles(filesWereAccounted);
    artworksRepository->watchFilePaths(watchList);
    artworksRepository->updateFilesCounts();

    std::unique_ptr<IHistoryItem> addArtworksItem(new AddArtworksHistoryItem(getCommandID(), ranges));
    auto *xpiks = commandManager->getDelegator();
    xpiks->recordHistoryItem(addArtworksItem);
    xpiks->generatePreviews(artworksToImport);

    int importID = commandManager->getDelegator()->readMetadata(artworksToImport);
    artItemsModel->raiseArtworksAdded(importID, usedCount, attachedVectors);

    if (!willResetModel) {
        artItemsModel->raiseArtworksChanged(true);
        artItemsModel->syncArtworksIndices();
    }

    commandManager->getDelegator()->saveSessionInBackground();

#ifndef CORE_TESTS
    MetadataIO::MetadataIOCoordinator *coordinator = commandManager->getMetadataIOCoordinator();
    if ((coordinator != nullptr) && coordinator->shouldUseAutoImport()) {
        LOG_DEBUG << "Autoimport artworks";
        coordinator->continueReading(false);
    }
#endif
}

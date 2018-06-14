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
#include <QVector>
#include <QHash>
#include <memory>
#include "addartworkscommand.h"
#include "commandmanager.h"
#include "../Models/artworksrepository.h"
#include "../Models/artworkmetadata.h"
#include "../Models/artitemsmodel.h"
#include "../UndoRedo/addartworksitem.h"
#include "../Common/defines.h"
#include "../Helpers/filehelpers.h"
#include "../Helpers/artworkshelpers.h"
#include "../Models/imageartwork.h"
#include "../MetadataIO/artworkssnapshot.h"
#include "../Models/settingsmodel.h"
#include "../Models/switchermodel.h"
#include "../MetadataIO/metadataiocoordinator.h"
#include "../Maintenance/maintenanceservice.h"

Commands::AddArtworksCommand::~AddArtworksCommand() {
    LOG_DEBUG << "#";
}

std::shared_ptr<Commands::ICommandResult> Commands::AddArtworksCommand::execute(const ICommandManager *commandManagerInterface) {
    LOG_INFO << m_FilePathes.length() << "images," << m_VectorsPathes.length() << "vectors";
    CommandManager *commandManager = (CommandManager*)commandManagerInterface;

    Models::ArtworksRepository *artworksRepository = commandManager->getArtworksRepository();
    Models::ArtItemsModel *artItemsModel = commandManager->getArtItemsModel();

    const int newFilesCount = artworksRepository->getNewFilesCount(m_FilePathes);
    const int initialCount = artItemsModel->rowCount();
    m_ArtworksToImport.reserve(newFilesCount);
    m_FilesToWatch.reserve(newFilesCount);

    LOG_INFO << "Current files count is" << initialCount;
    addFilesToAdd(newFilesCount, artworksRepository, artItemsModel);

    artworksRepository->watchFilePaths(m_FilesToWatch);
    artworksRepository->updateFilesCounts();

    QHash<QString, QHash<QString, QString> > vectorsHash;
    decomposeVectors(vectorsHash);
    QVector<int> modifiedIndices;

    int attachedCount = artItemsModel->attachVectors(vectorsHash, modifiedIndices);



    int importID = 0;

    if (newFilesCount > 0) {
        importID = afterAddedHandler(commandManager, m_ArtworksToImport, m_FilesToWatch, initialCount, newFilesCount);
    }

    artItemsModel->updateItems(modifiedIndices, );

    std::shared_ptr<AddArtworksCommandResult> result(new AddArtworksCommandResult(
                                                         *artworksRepository,
                                                         m_DirectoryIDs,
                                                         newFilesCount,
                                                         attachedCount,
                                                         importID,
                                                         getAutoImportFlag()));
    return result;
}

int Commands::AddArtworksCommand::afterAddedHandler(CommandManager *commandManager, const MetadataIO::ArtworksSnapshot &artworksToImport, QStringList filesToWatch, int initialCount, int newFilesCount) const {
    Models::ArtworksRepository *artworksRepository = commandManager->getArtworksRepository();
    auto *xpiks = commandManager->getDelegator();

    int importID = xpiks->readMetadata(artworksToImport);
    accountVectors(artworksRepository, artworksToImport.getWeakSnapshot());
    artworksRepository->refresh();



    // Generating previews was in the metadata io coordinator
    // called _after_ the reading to make reading (in Xpiks)
    // as fast as possible. Not needed if using only exiftool now
    xpiks->generatePreviews(artworksToImport);
    xpiks->addToRecentFiles(filesToWatch);

    if (!getIsSessionRestoreFlag()) {
        xpiks->saveSessionInBackground();
    }

    return importID;
}



void Commands::AddArtworksCommandResult::afterExecCallback(const Commands::ICommandManager *commandManagerInterface) {
    CommandManager *commandManager = (CommandManager*)commandManagerInterface;

#ifndef CORE_TESTS
    if (m_AutoImport) {
        LOG_DEBUG << "Autoimport is ON. Proceeding...";
        MetadataIO::MetadataIOCoordinator *ioCoordinator = commandManager->getMetadataIOCoordinator();
        ioCoordinator->continueReading(false);
        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
    }
#endif

    Models::ArtItemsModel *artItemsModel = commandManager->getArtItemsModel();
    artItemsModel->raiseArtworksAdded(m_ImportID, m_NewFilesAdded, m_AttachedVectorsCount);

#ifndef CORE_TESTS
    Maintenance::MaintenanceService *maintenanceService = commandManager->getMaintenanceService();
    QString directoryPath;
    for (auto &id: m_DirectoryIDs) {
        if (m_ArtworksRepository.tryGetDirectoryPath(id, directoryPath)) {
            maintenanceService->cleanupOldXpksBackups(directoryPath);
        }
    }
#endif
}

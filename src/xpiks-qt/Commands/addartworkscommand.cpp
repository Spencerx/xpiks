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

void accountVectors(Models::ArtworksRepository *artworksRepository, const MetadataIO::WeakArtworksSnapshot &artworks) {
    LOG_DEBUG << "#";

    size_t size = artworks.size();
    for (size_t i = 0; i < size; ++i) {
        Models::ArtworkMetadata *metadata = artworks.at(i);
        Models::ImageArtwork *imageArtwork = dynamic_cast<Models::ImageArtwork *>(metadata);
        if ((imageArtwork != nullptr) && imageArtwork->hasVectorAttached()) {
            artworksRepository->accountVector(imageArtwork->getAttachedVectorPath());
        }
    }
}

Commands::AddArtworksCommand::~AddArtworksCommand() {
    LOG_DEBUG << "#";
}

std::shared_ptr<Commands::ICommandResult> Commands::AddArtworksCommand::execute(const ICommandManager *commandManagerInterface) const {
    LOG_INFO << m_FilePathes.length() << "images," << m_VectorsPathes.length() << "vectors";
    CommandManager *commandManager = (CommandManager*)commandManagerInterface;

    Models::ArtworksRepository *artworksRepository = commandManager->getArtworksRepository();
    Models::ArtItemsModel *artItemsModel = commandManager->getArtItemsModel();

    const int newFilesCount = artworksRepository->getNewFilesCount(m_FilePathes);
    const int initialCount = artItemsModel->rowCount();
    const bool filesWereAccounted = artworksRepository->beginAccountingFiles(m_FilePathes);

    MetadataIO::ArtworksSnapshot artworksToImport;
    artworksToImport.reserve(newFilesCount);
    QStringList filesToWatch;
    filesToWatch.reserve(newFilesCount);

    if (newFilesCount > 0) {
        LOG_INFO << newFilesCount << "new files found";
        LOG_INFO << "Current files count is" << initialCount;
        artItemsModel->beginAccountingFiles(newFilesCount);

        const int count = m_FilePathes.count();
        Common::DirectoryFlags directoryFlags = Common::DirectoryFlags::None;
        if (getIsFullDirectoryFlag()) { Common::SetFlag(directoryFlags, Common::DirectoryFlags::IsAddedAsDirectory); }

        for (int i = 0; i < count; ++i) {
            const QString &filename = m_FilePathes[i];
            qint64 directoryID = 0;

            if (artworksRepository->accountFile(filename, directoryID, directoryFlags)) {
                Models::ArtworkMetadata *artwork = artItemsModel->createArtwork(filename, directoryID);

                LOG_INTEGRATION_TESTS << "Added file:" << filename;

                artItemsModel->appendArtwork(artwork);
                artworksToImport.append(artwork);
                filesToWatch.append(filename);
            } else {
                LOG_INFO << "Rejected file:" << filename;
            }
        }

        artItemsModel->endAccountingFiles();
    }

    artworksRepository->endAccountingFiles(filesWereAccounted);
    artworksRepository->watchFilePaths(filesToWatch);
    artworksRepository->updateFilesCounts();

    QHash<QString, QHash<QString, QString> > vectorsHash;
    decomposeVectors(vectorsHash);
    QVector<int> modifiedIndices;

    int attachedCount = artItemsModel->attachVectors(vectorsHash, modifiedIndices);

    if (getAutoFindVectorsFlag()) {
        QVector<int> autoAttachedIndices;
        attachedCount = Helpers::findAndAttachVectors(artworksToImport.getWeakSnapshot(), autoAttachedIndices);

        foreach (int index, autoAttachedIndices) {
            modifiedIndices.append(initialCount + index);
        }
    }

    int importID = 0;

    if (newFilesCount > 0) {
        importID = afterAddedHandler(commandManager, artworksToImport, filesToWatch, initialCount, newFilesCount);
    }

    artItemsModel->updateItems(modifiedIndices, QVector<int>() << Models::ArtItemsModel::HasVectorAttachedRole);

    std::shared_ptr<AddArtworksCommandResult> result(new AddArtworksCommandResult(
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

    std::unique_ptr<UndoRedo::IHistoryItem> addArtworksItem(new UndoRedo::AddArtworksHistoryItem(getCommandID(), initialCount, newFilesCount));
    xpiks->recordHistoryItem(addArtworksItem);

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

void Commands::AddArtworksCommand::decomposeVectors(QHash<QString, QHash<QString, QString> > &vectors) const {
    int size = m_VectorsPathes.size();
    LOG_DEBUG << size << "item(s)";

    for (int i = 0; i < size; ++i) {
        const QString &path = m_VectorsPathes.at(i);
        QFileInfo fi(path);
        const QString &absolutePath = fi.absolutePath();

        if (!vectors.contains(absolutePath)) {
            vectors.insert(absolutePath, QHash<QString, QString>());
        }

        vectors[absolutePath].insert(fi.baseName().toLower(), path);
    }
}

void Commands::AddArtworksCommandResult::afterExecCallback(const Commands::ICommandManager *commandManagerInterface) const {
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
}

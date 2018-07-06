/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "metadataiocoordinator.h"
#include <QHash>
#include <QFileInfo>
#include <QProcess>
#include <QImageReader>
#include <Artworks/artworkmetadata.h>
#include <Artworks/imageartwork.h>
#include "../Models/settingsmodel.h"
#include "../Common/defines.h"
#include "metadataioservice.h"
#include <readingorchestrator.h>
#include <writingorchestrator.h>
#include "../Models/switchermodel.h"
#include <Services/artworksupdatehub.h>
#include "../QMLExtensions/videocachingservice.h"

namespace MetadataIO {
    void lockForIO(const Artworks::ArtworksSnapshot &snapshot) {
        const size_t size = snapshot.size();
        LOG_DEBUG << size << "item(s)";

        for (size_t i = 0; i < size; i++) {
            Artworks::ArtworkMetadata *artwork = snapshot.get(i);
            artwork->setIsLockedIO(true);
        }
    }

    MetadataIOCoordinator::MetadataIOCoordinator(MetadataReadingHub &readingHub,
                                                 Models::SettingsModel &settingsModel,
                                                 Models::SwitcherModel &switcherModel,
                                                 QMLExtensions::VideoCachingService &videoCachingService):
        m_ReadingHub(readingHub),
        m_SettingsModel(settingsModel),
        m_SwitcherModel(switcherModel),
        m_VideoCachingService(videoCachingService),
        m_LastImportID(1),
        m_ProcessingItemsCount(0),
        m_IsInProgress(false),
        m_HasErrors(false),
        m_ExiftoolNotFound(false)
    {
        LOG_INFO << "Supported image formats:" << QImageReader::supportedImageFormats();

        QObject::connect(&m_WritingAsyncCoordinator, &Helpers::AsyncCoordinator::statusReported,
                         this, &MetadataIOCoordinator::writingWorkersFinished);

        QObject::connect(&m_ReadingHub, &MetadataReadingHub::readingFinished,
                         this, &MetadataIOCoordinator::onReadingFinished);
    }

    bool MetadataIOCoordinator::shouldUseAutoImport() const {
        bool autoImport = false;

        if (m_SettingsModel.getUseAutoImport() && m_SwitcherModel.getUseAutoImport()) {
            autoImport = true;
        }

        return autoImport;
    }

    int MetadataIOCoordinator::readMetadataExifTool(const Artworks::ArtworksSnapshot &artworksToRead, quint32 storageReadBatchID) {
        int importID = getNextImportID();
        initializeImport(artworksToRead, importID, storageReadBatchID);

        libxpks::io::ReadingOrchestrator readingOrchestrator(&m_ReadingHub,
                                                             &m_SettingsModel);
        readingOrchestrator.startReading();

        return importID;
    }

    void MetadataIOCoordinator::writeMetadataExifTool(const Artworks::ArtworksSnapshot &artworksToWrite, bool useBackups) {
        LOG_DEBUG << "use backups:" << useBackups;
        m_WritingAsyncCoordinator.reset();

        lockForIO(artworksToWrite);

        // this should prevent a race between video thumbnails and exiftool
        // https://github.com/ribtoks/xpiks/issues/477
        // ---
        m_VideoCachingService.waitWorkerIdle();
        // ---

        libxpks::io::WritingOrchestrator writingOrchestrator(artworksToWrite,
                                                             &m_WritingAsyncCoordinator,
                                                             &m_SettingsModel);

#ifndef INTEGRATION_TESTS
        const bool directExportOn = m_SwitcherModel.getUseDirectMetadataExport();
#else
        const bool directExportOn = false;
#endif

        const bool useDirectExport = m_SettingsModel.getUseDirectExiftoolExport();

        writingOrchestrator.startWriting(useBackups, useDirectExport || directExportOn);
    }

    void MetadataIOCoordinator::wipeAllMetadataExifTool(const Artworks::ArtworksSnapshot &artworksToWipe, bool useBackups) {
        LOG_DEBUG << "use backups:" << useBackups;
        m_WritingAsyncCoordinator.reset();

        lockForIO(artworksToWipe);

        // this should prevent a race between video thumbnails and exiftool
        // https://github.com/ribtoks/xpiks/issues/477
        // ---
        m_VideoCachingService.waitWorkerIdle();
        // ---

        libxpks::io::WritingOrchestrator writingOrchestrator(artworksToWipe,
                                                             &m_WritingAsyncCoordinator,
                                                             &m_SettingsModel);

        writingOrchestrator.startMetadataWiping(useBackups);
    }

    void MetadataIOCoordinator::continueReading(bool ignoreBackups) {
        LOG_DEBUG << "ignore backups:" << ignoreBackups;
        setIsInProgress(true);
        m_ReadingHub.proceedImport(ignoreBackups);
    }

    void MetadataIOCoordinator::continueWithoutReading(bool ignoreBackups, bool reimport) {
        LOG_INFO << "ignore backups:" << ignoreBackups << "reimport:" << reimport;
        setIsInProgress(true);
        if (!reimport) {
            m_ReadingHub.cancelImport(ignoreBackups);
        } else {
            m_ReadingHub.skipImport();
        }
    }

    bool MetadataIOCoordinator::hasImportFinished(int importID) {
        Q_ASSERT(m_PreviousImportIDs.find(0) == m_PreviousImportIDs.end());
        if (importID == 0) { return false; }
        bool found = m_PreviousImportIDs.find(importID) != m_PreviousImportIDs.end();
        return found;
    }

    void MetadataIOCoordinator::writingWorkersFinished(int status) {
        LOG_DEBUG << status;

        emit metadataWritingFinished();
    }

    void MetadataIOCoordinator::onReadingFinished(int importID) {
        LOG_DEBUG << "import #" << importID;

        m_PreviousImportIDs.insert(importID);
        emit metadataReadingFinished();
        setIsInProgress(false);
    }

    void MetadataIOCoordinator::onRecommendedExiftoolFound(const QString &path) {
        LOG_DEBUG << path;
        setExiftoolNotFound(path.isEmpty());
    }

    int MetadataIOCoordinator::getNextImportID() {
        int id = m_LastImportID++;
        return id;
    }

    void MetadataIOCoordinator::initializeImport(const Artworks::ArtworksSnapshot &artworksToRead, int importID, quint32 storageReadBatchID) {
        m_ReadingHub.initializeImport(artworksToRead, importID, storageReadBatchID);

        setHasErrors(false);
        setIsInProgress(false);
        setProcessingItemsCount((int)artworksToRead.size());
    }
}

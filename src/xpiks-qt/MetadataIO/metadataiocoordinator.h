/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef METADATAIOCOORDINATOR_H
#define METADATAIOCOORDINATOR_H

#include <QObject>
#include <QVector>
#include <QAtomicInt>
#include <set>
#include "../Common/defines.h"
#include "../Common/readerwriterqueue.h"
#include "originalmetadata.h"
#include "metadatareadinghub.h"

namespace Artworks {
    class ArtworkMetadata;
    class ArtworksSnapshot;
}

namespace Models {
    class SettingsModel;
    class SwitcherModel;
}

namespace QMLExtensions {
    class VideoCachingService;
}

namespace MetadataIO {
    class MetadataWritingWorker;

    class MetadataIOCoordinator : public QObject
    {
        Q_OBJECT
        Q_PROPERTY(int processingItemsCount READ getProcessingItemsCount WRITE setProcessingItemsCount NOTIFY processingItemsCountChanged)
        Q_PROPERTY(bool hasErrors READ getHasErrors WRITE setHasErrors NOTIFY hasErrorsChanged)
        Q_PROPERTY(bool exiftoolNotFound READ getExiftoolNotFound WRITE setExiftoolNotFound NOTIFY exiftoolNotFoundChanged)
        Q_PROPERTY(bool isInProgress READ getIsInProgress WRITE setIsInProgress NOTIFY isInProgressChanged)

    public:
        MetadataIOCoordinator(MetadataReadingHub &readingHub,
                              Models::SettingsModel &settingsModel,
                              Models::SwitcherModel &switcherModel,
                              QMLExtensions::VideoCachingService &videoCachingService);

    signals:
        void metadataReadingFinished();
        void metadataWritingFinished();
        void processingItemsCountChanged(int value);
        void hasErrorsChanged(bool value);
        void exiftoolNotFoundChanged();
        void isInProgressChanged();
        void recommendedExiftoolFound(const QString &path);

    public:
        bool getExiftoolNotFound() const { return m_ExiftoolNotFound; }
        int getProcessingItemsCount() const { return m_ProcessingItemsCount; }
        bool getHasErrors() const { return m_HasErrors; }
        bool getIsInProgress() const { return m_IsInProgress; }

    public:
        void setExiftoolNotFound(bool value) {
            if (value != m_ExiftoolNotFound) {
                LOG_INFO << value;
                m_ExiftoolNotFound = value;
                emit exiftoolNotFoundChanged();
            }
        }

        void setProcessingItemsCount(int value) {
            if (value != m_ProcessingItemsCount) {
                m_ProcessingItemsCount = value;
                emit processingItemsCountChanged(value);
            }
        }

        void setHasErrors(bool value) {
            if (value != m_HasErrors) {
                m_HasErrors = value;
                emit hasErrorsChanged(value);
            }
        }

        void setIsInProgress(bool value) {
            if (value != m_IsInProgress) {
                m_IsInProgress = value;
                emit isInProgressChanged();
            }
        }

    public:
        int readMetadataExifTool(const Artworks::ArtworksSnapshot &artworksToRead, quint32 storageReadBatchID);
        void writeMetadataExifTool(const Artworks::ArtworksSnapshot &artworksToWrite, bool useBackups);
        void wipeAllMetadataExifTool(const Artworks::ArtworksSnapshot &artworksToWipe, bool useBackups);

#ifdef INTEGRATION_TESTS
    public:
        const std::set<int> &getImportIDs() const { return m_PreviousImportIDs; }
        void clear() { m_PreviousImportIDs.clear(); }
#endif

    public:
        Q_INVOKABLE bool shouldUseAutoImport() const;
        Q_INVOKABLE void continueReading(bool ignoreBackups);
        Q_INVOKABLE void continueWithoutReading(bool ignoreBackups, bool reimport = false);
        Q_INVOKABLE bool hasImportFinished(int importID);

    private slots:
        void writingWorkersFinished(int status);
        void onReadingFinished(int importID);

    public slots:
        void onRecommendedExiftoolFound(const QString &path);

    private:
        int getNextImportID();
        void initializeImport(const Artworks::ArtworksSnapshot &artworksToRead, int importID, quint32 storageReadBatchID);
        void readingFinishedHandler(bool ignoreBackups);
        void afterImportHandler(const QVector<Artworks::ArtworkMetadata*> &itemsToRead, bool ignoreBackups);

    private:
        Helpers::AsyncCoordinator m_WritingAsyncCoordinator;
        MetadataReadingHub &m_ReadingHub;
        Models::SettingsModel &m_SettingsModel;
        Models::SwitcherModel &m_SwitcherModel;
        QMLExtensions::VideoCachingService &m_VideoCachingService;
        int m_LastImportID;
        std::set<int> m_PreviousImportIDs;
        volatile int m_ProcessingItemsCount;
        volatile bool m_IsInProgress;
        volatile bool m_HasErrors;
        volatile bool m_ExiftoolNotFound;
    };
}

#endif // METADATAIOCOORDINATOR_H

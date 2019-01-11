/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef METADATAIOCOORDINATOR_H
#define METADATAIOCOORDINATOR_H

#include <atomic>
#include <set>

#include <QObject>
#include <QString>
#include <QtGlobal>

#include "Helpers/asynccoordinator.h"

template <typename T> class QVector;

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
    class MetadataReadingHub;

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
        void setExiftoolNotFound(bool value);
        void setProcessingItemsCount(int value);
        void setHasErrors(bool value);
        void setIsInProgress(bool value);

    public:
        int readMetadataExifTool(Artworks::ArtworksSnapshot const &artworksToRead, quint32 storageReadBatchID);
        void reimportMetadataExiftool(Artworks::ArtworksSnapshot const &artworksToRead);
        void writeMetadataExifTool(Artworks::ArtworksSnapshot const &artworksToWrite, bool useBackups);
        void wipeAllMetadataExifTool(Artworks::ArtworksSnapshot const &artworksToWipe, bool useBackups);

#if defined(INTEGRATION_TESTS) || defined(UI_TESTS)
    public:
        const std::set<int> &getImportIDs() const { return m_PreviousImportIDs; }
        void clear() { m_PreviousImportIDs.clear(); }
#endif

    public:
        Q_INVOKABLE bool shouldUseAutoImport() const;
        Q_INVOKABLE void continueReading(bool ignoreBackups);
        Q_INVOKABLE void continueWithoutReading(bool ignoreBackups, bool reimport = false);
        Q_INVOKABLE bool hasImportFinished(int importID);

    signals:
        void importStarted(int importID, bool reimport);
        void importFinished(int importID);

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
        std::atomic_bool m_IsInProgress;
        std::atomic_bool m_HasErrors;
        std::atomic_bool m_ExiftoolNotFound;
    };
}

#endif // METADATAIOCOORDINATOR_H

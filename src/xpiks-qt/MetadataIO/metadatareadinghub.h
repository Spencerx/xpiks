/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef METADATAREADINGHUB_H
#define METADATAREADINGHUB_H

#include <QObject>
#include <QAtomicInt>
#include <Common/readerwriterqueue.h>
#include <Artworks/artworkssnapshot.h>
#include "originalmetadata.h"
#include <Helpers/asynccoordinator.h>

namespace Services {
    class ArtworksUpdateHub;
    class ArtworksInspectionHub;
}

namespace MetadataIO {
    class MetadataIOService;

    class MetadataReadingHub: public QObject
    {
        Q_OBJECT
    public:
        MetadataReadingHub(MetadataIOService &metadataIOService,
                           Services::ArtworksUpdateHub &updateHub,
                           Services::ArtworksInspectionHub &inspectionHub);

    public:
        Artworks::ArtworksSnapshot const &getSnapshot() const { return m_ArtworksToRead; }

    public:
        void initializeImport(Artworks::ArtworksSnapshot const &artworksToRead, int importID, quint32 storageReadBatchID);
        void finalizeImport();

    public:
        void accountReadIO();
        void startAcceptingIOResults();

    public:
        void proceedImport(bool ignoreBackups);
        void cancelImport(bool ignoreBackups);
        void skipImport();

    public:
        void push(std::shared_ptr<OriginalMetadata> &item);

    private slots:
        void onCanInitialize(int status);

    signals:
        void readingFinished(int importID);

    private:
        void initializeArtworks(bool ignoreBackups, bool isCancelled);

    private:
        Artworks::ArtworksSnapshot m_ArtworksToRead;
        Helpers::AsyncCoordinator m_AsyncCoordinator;
        MetadataIOService &m_MetadataIOService;
        Services::ArtworksUpdateHub &m_UpdateHub;
        Services::ArtworksInspectionHub &m_InspectionHub;
        Common::ReaderWriterQueue<OriginalMetadata> m_ImportQueue;
        int m_ImportID;
        quint32 m_StorageReadBatchID;
        volatile bool m_IgnoreBackupsAtImport;
        volatile bool m_IsCancelled;
    };
}

#endif // METADATAREADINGHUB_H

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

#include <memory>

#include <QObject>
#include <QString>
#include <QtGlobal>

#include "Artworks/artworkssnapshot.h"
#include "Common/readerwriterqueue.h"
#include "Helpers/asynccoordinator.h"

namespace Services {
    class ArtworksEditingHub;
    class ArtworksUpdateHub;
}

namespace MetadataIO {
    class MetadataIOService;
    struct OriginalMetadata;

    class MetadataReadingHub: public QObject
    {
        Q_OBJECT
    public:
        MetadataReadingHub(MetadataIOService &metadataIOService,
                           Services::ArtworksUpdateHub &updateHub,
                           Services::ArtworksEditingHub &inspectionHub);

    public:
        Artworks::ArtworksSnapshot const &getSnapshot() const { return m_ArtworksToRead; }

    public:
        void initializeImport(Artworks::ArtworksSnapshot const &artworksToRead, int importID, quint32 storageReadBatchID);
        void finalizeImport();

    public:
        void accountReadIO();
        void startAcceptingIOResults();
        std::shared_ptr<Helpers::AsyncCoordinatorUnlocker> getIOFinalizer();

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
        Services::ArtworksEditingHub &m_InspectionHub;
        Common::ReaderWriterQueue<OriginalMetadata> m_ImportQueue;
        int m_ImportID;
        quint32 m_StorageReadBatchID;
        volatile bool m_IgnoreBackupsAtImport;
        volatile bool m_IsCancelled;
    };
}

#endif // METADATAREADINGHUB_H

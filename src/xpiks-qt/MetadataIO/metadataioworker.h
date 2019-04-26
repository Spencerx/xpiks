/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef METADATAIOWORKER_H
#define METADATAIOWORKER_H

#include <memory>
#include <mutex>
#include <unordered_set>

#include <QObject>
#include <QString>

#include "Common/itemprocessingworker.h"
#include "Common/readerwriterqueue.h"
#include "MetadataIO/cachedartwork.h"
#include "MetadataIO/metadatacache.h"

namespace Artworks {
    class ArtworkMetadata;
}

namespace Services {
    class ArtworksUpdateHub;
}

namespace Storage {
    class IDatabaseManager;
}

namespace MetadataIO {
    class MetadataIOTaskBase;
    class MetadataReadWriteTask;
    class MetadataSearchTask;

    struct StorageReadRequest {
        StorageReadRequest(uint32_t batchID):
            m_BatchID(batchID)
        {}
        CachedArtwork m_CachedArtwork;
        std::shared_ptr<Artworks::ArtworkMetadata> m_Artwork;
        uint32_t m_BatchID;
    };

    class MetadataIOWorker : public QObject, public Common::ItemProcessingWorker<MetadataIOTaskBase>
    {
        Q_OBJECT
    public:
        explicit MetadataIOWorker(Storage::IDatabaseManager &dbManager,
                                  Services::ArtworksUpdateHub &artworksUpdateHub,
                                  QObject *parent = nullptr);

#ifdef INTEGRATION_TESTS
    public:
        MetadataCache &getMetadataCache() { return m_MetadataCache; }
#endif

    public:
        virtual void cancelBatch(batch_id_t batchID) override;

    protected:
        virtual bool initWorker() override;
        virtual std::shared_ptr<void> processWorkItem(WorkItem &workItem) override;
        void processIOItem(const std::shared_ptr<MetadataIOTaskBase> &item, batch_id_t batchID);

    private:
        void processReadWriteItem(const std::shared_ptr<MetadataReadWriteTask> &item, batch_id_t batchID);
        void processSearchItem(const std::shared_ptr<MetadataSearchTask> &item);

    public:
        void importArtworksFromStorage();

    protected:
        virtual void onQueueIsEmpty() override { emit queueIsEmpty(); }
        virtual void onWorkerStopped() override;

    public slots:
        void process() { doWork(); }
        void cancel() { stopWorking(); }

    signals:
        void stopped();
        void queueIsEmpty();
        void readyToImportFromStorage();

    private:
        Common::ReaderWriterQueue<StorageReadRequest> m_StorageReadQueue;
        Services::ArtworksUpdateHub &m_ArtworksUpdateHub;
        MetadataCache m_MetadataCache;
        std::mutex m_CancelMutex;
        std::unordered_set<batch_id_t> m_CancelledImports;
        volatile int m_ProcessedItemsCount;
    };
}

#endif // METADATAIOWORKER_H

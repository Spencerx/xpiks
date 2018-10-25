/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef METADATAIOWORKER_H
#define METADATAIOWORKER_H

#include <memory>

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
        CachedArtwork m_CachedArtwork;
        std::shared_ptr<Artworks::ArtworkMetadata> m_Artwork;
    };

    class MetadataIOWorker : public QObject, public Common::ItemProcessingWorker<MetadataIOTaskBase>
    {
        Q_OBJECT
    public:
        explicit MetadataIOWorker(Storage::IDatabaseManager &dbManager,
                                  Services::ArtworksUpdateHub &artworksUpdateHub,
                                  QObject *parent = 0);

#ifdef INTEGRATION_TESTS
    public:
        MetadataCache &getMetadataCache() { return m_MetadataCache; }
#endif

    protected:
        virtual bool initWorker() override;
        virtual std::shared_ptr<void> processWorkItem(WorkItem &workItem) override;
        virtual void processOneItem(std::shared_ptr<MetadataIOTaskBase> &item) override;

    private:
        void processReadWriteItem(std::shared_ptr<MetadataReadWriteTask> &item);
        void processSearchItem(std::shared_ptr<MetadataSearchTask> &item);

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
        volatile int m_ProcessedItemsCount;
    };
}

#endif // METADATAIOWORKER_H

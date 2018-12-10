/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "metadataioworker.h"

#include <vector>

#include <QtDebug>
#include <QtGlobal>

#include "Artworks/artworkmetadata.h"
#include "Common/itemprocessingworker.h"
#include "Common/logging.h"
#include "Common/readerwriterqueue.h"
#include "MetadataIO/metadatacache.h"
#include "MetadataIO/metadataiotask.h"
#include "Services/artworksupdatehub.h"
#include "Suggestion/locallibraryquery.h"

#define METADATA_CACHE_SYNC_INTERVAL 29
#define STORAGE_IMPORT_INTERVAL 29

namespace MetadataIO {
    MetadataIOWorker::MetadataIOWorker(Storage::IDatabaseManager &dbManager,
                                       Services::ArtworksUpdateHub &artworksUpdateHub,
                                       QObject *parent):
        QObject(parent),
        m_ArtworksUpdateHub(artworksUpdateHub),
        m_MetadataCache(dbManager),
        m_ProcessedItemsCount(0)
    {
    }

    void MetadataIOWorker::cancelBatch(batch_id_t batchID) {
        LOG_DEBUG << batchID;
        Common::ItemProcessingWorker<MetadataIOTaskBase>::cancelBatch(batchID);
        {
            std::lock_guard<std::mutex> lock(m_CancelMutex);
            m_CancelledImports.insert(batchID);
        }
    }

    bool MetadataIOWorker::initWorker() {
        LOG_DEBUG << "#";
        bool success = m_MetadataCache.initialize();
        if (!success) {
            LOG_WARNING << "Failed to initialize metadata cache";
        }

        return true;
    }

    std::shared_ptr<void> MetadataIOWorker::processWorkItem(WorkItem &workItem) {
        if (workItem.isSeparator()) {
            LOG_DEBUG << "Processing separator";
            m_MetadataCache.sync();
            emit readyToImportFromStorage();
        } else {
            processIOItem(workItem.m_Item, workItem.m_ID);
        }

        return std::shared_ptr<void>();
    }

    void MetadataIOWorker::processIOItem(std::shared_ptr<MetadataIOTaskBase> &item, batch_id_t batchID) {
        do {
            std::shared_ptr<MetadataReadWriteTask> readWriteItem = std::dynamic_pointer_cast<MetadataReadWriteTask>(item);
            if (readWriteItem) {
                processReadWriteItem(readWriteItem, batchID);
                break;
            }

            std::shared_ptr<MetadataSearchTask> searchTask = std::dynamic_pointer_cast<MetadataSearchTask>(item);
            if (searchTask) {
                processSearchItem(searchTask);
                break;
            }

            LOG_WARNING << "Unknown task";
            Q_ASSERT(false);
        } while(false);
    }

    void MetadataIOWorker::processReadWriteItem(std::shared_ptr<MetadataReadWriteTask> &item, batch_id_t batchID) {
        auto &artwork = item->getArtworkMetadata();
        Q_ASSERT(artwork != nullptr);
        if (artwork == nullptr) { return; }

        const MetadataReadWriteTask::ReadWriteAction action = item->getReadWriteAction();
        if (action == MetadataReadWriteTask::Read) {
            auto readRequest = std::make_shared<StorageReadRequest>(batchID);
            if (m_MetadataCache.read(artwork, readRequest->m_CachedArtwork)) {
                readRequest->m_Artwork = artwork;
                m_StorageReadQueue.push(readRequest);
            }
        } else if (action == MetadataReadWriteTask::Write) {
            m_MetadataCache.save(artwork, true);
        } else if (action == MetadataReadWriteTask::Add) {
            m_MetadataCache.save(artwork, false);
        }

        m_ProcessedItemsCount++;

        if (m_ProcessedItemsCount % METADATA_CACHE_SYNC_INTERVAL == 0) {
            m_MetadataCache.sync();
        }

        if (m_StorageReadQueue.size() > STORAGE_IMPORT_INTERVAL) {
            emit readyToImportFromStorage();
        }
    }

    void MetadataIOWorker::processSearchItem(std::shared_ptr<MetadataSearchTask> &item) {
        auto *localLibraryQuery = item->getQuery();
        m_MetadataCache.search(localLibraryQuery->getSearchQuery(), localLibraryQuery->getResults());
        localLibraryQuery->notifyResultsReady();
    }

    void MetadataIOWorker::importArtworksFromStorage() {
        LOG_DEBUG << "#";
        std::vector<std::shared_ptr<StorageReadRequest>> readRequests;
        // popAll() returns queue in reversed order for performance reasons
        m_StorageReadQueue.popAll(readRequests);
        LOG_DEBUG << readRequests.size() << "requests to process";

        std::lock_guard<std::mutex> cancelLock(m_CancelMutex);
        auto itEnd = m_CancelledImports.end();

        for (auto &request: readRequests) {
            if (m_CancelledImports.find(request->m_BatchID) == itEnd) {
                bool modified = request->m_Artwork->initFromStorage(request->m_CachedArtwork);
                Q_UNUSED(modified);

                m_ArtworksUpdateHub.updateArtwork(request->m_Artwork);
            } else {
                LOG_VERBOSE << "Skipping cancelled artwork" << request->m_Artwork->getItemID();
            }
        }
    }

    void MetadataIOWorker::onWorkerStopped() {
        m_MetadataCache.finalize();
        emit stopped();
    }
}

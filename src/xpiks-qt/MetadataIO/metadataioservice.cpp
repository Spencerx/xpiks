/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "metadataioservice.h"

#include <vector>

#include <QThread>
#include <QtDebug>

#include "Artworks/artworkmetadata.h"
#include "Common/itemprocessingworker.h"
#include "Common/logging.h"
#include "MetadataIO/metadataiotask.h"
#include "MetadataIO/metadataioworker.h"

namespace Suggestion {
    class LocalLibraryQuery;
}

#define SAVER_TIMER_TIMEOUT 2000
#define SAVER_TIMER_MAX_RESTARTS 5

namespace MetadataIO {
    MetadataIOService::MetadataIOService(QObject *parent):
        QObject(parent),
        Common::DelayedActionEntity(SAVER_TIMER_TIMEOUT, SAVER_TIMER_MAX_RESTARTS),
        m_MetadataIOWorker(nullptr),
        m_IsStopped(false)
    {
        // timers could not be started from another thread
        QObject::connect(this, &MetadataIOService::cacheSyncRequest, this, &MetadataIOService::onCacheSyncRequest);
    }

    void MetadataIOService::startService(Storage::IDatabaseManager &databaseManager,
                                         Services::ArtworksUpdateHub &artworksUpdateHub) {
        Q_ASSERT(m_MetadataIOWorker == nullptr);

        m_MetadataIOWorker = new MetadataIOWorker(databaseManager, artworksUpdateHub);

        QThread *thread = new QThread();
        m_MetadataIOWorker->moveToThread(thread);

        QObject::connect(thread, &QThread::started, m_MetadataIOWorker, &MetadataIOWorker::process);
        QObject::connect(m_MetadataIOWorker, &MetadataIOWorker::stopped, thread, &QThread::quit);

        QObject::connect(m_MetadataIOWorker, &MetadataIOWorker::stopped, m_MetadataIOWorker, &MetadataIOWorker::deleteLater);
        QObject::connect(thread, &QThread::finished, thread, &QThread::deleteLater);

        QObject::connect(m_MetadataIOWorker, &MetadataIOWorker::stopped,
                         this, &MetadataIOService::workerFinished);

        QObject::connect(m_MetadataIOWorker, &MetadataIOWorker::readyToImportFromStorage,
                         this, &MetadataIOService::onReadyToImportFromStorage);

        thread->start();

        m_IsStopped = false;
    }

    void MetadataIOService::stopService() {
        LOG_DEBUG << "#";
        Q_ASSERT(m_MetadataIOWorker != nullptr);
        m_MetadataIOWorker->stopWorking();
        m_IsStopped = true;
    }

    void MetadataIOService::cancelBatch(quint32 batchID) const {
        LOG_INFO << batchID;
        Q_ASSERT(m_MetadataIOWorker != nullptr);
        m_MetadataIOWorker->cancelBatch(batchID);
    }

    bool MetadataIOService::isBusy() const {
        if (m_IsStopped) { return false; }
        if (m_MetadataIOWorker == nullptr) { return false; }
        return m_MetadataIOWorker->hasPendingJobs();
    }

    void MetadataIOService::waitWorkerIdle() {
        LOG_DEBUG << "#";
        Q_ASSERT(m_MetadataIOWorker != nullptr);
        m_MetadataIOWorker->waitIdle();
    }

    void MetadataIOService::writeArtwork(std::shared_ptr<Artworks::ArtworkMetadata> const &artwork) {
        Q_ASSERT(artwork != nullptr);
        if (m_IsStopped) { return; }
        LOG_DEBUG << "Saving" << artwork->getItemID();

        auto jobItem = std::make_shared<MetadataReadWriteTask>(artwork, MetadataReadWriteTask::Write);
        m_MetadataIOWorker->submitItem(jobItem);

        emit cacheSyncRequest();
    }

    quint32 MetadataIOService::readArtworks(const Artworks::ArtworksSnapshot &snapshot) const {
        LOG_INFO << snapshot.size() << "artwork(s)";
        if (m_IsStopped) { return 0; }
        std::vector<std::shared_ptr<MetadataIOTaskBase> > jobs;
        jobs.reserve(snapshot.size());

        for (auto &artwork: snapshot) {
            jobs.emplace_back(std::make_shared<MetadataReadWriteTask>(artwork, MetadataReadWriteTask::Read));
        }

        MetadataIOWorker::batch_id_t batchID = m_MetadataIOWorker->submitItems(jobs);
        m_MetadataIOWorker->submitSeparator();
        LOG_INFO << "Batch ID is" << batchID;

        return batchID;
    }

    void MetadataIOService::writeArtworks(const Artworks::ArtworksSnapshot &snapshot) const {
        LOG_INFO << snapshot.size() << "artwork(s)";
        if (m_IsStopped) { return; }
        std::vector<std::shared_ptr<MetadataIOTaskBase> > jobs;
        jobs.reserve(snapshot.size());

        for (auto &artwork: snapshot) {
            jobs.emplace_back(std::make_shared<MetadataReadWriteTask>(artwork, MetadataReadWriteTask::Write));
        }

        m_MetadataIOWorker->submitItems(jobs);
        m_MetadataIOWorker->submitSeparator();
    }

    void MetadataIOService::addArtworks(const Artworks::ArtworksSnapshot &snapshot) const {
        LOG_INFO << snapshot.size() << "artwork(s)";
        if (m_IsStopped) { return; }
        std::vector<std::shared_ptr<MetadataIOTaskBase> > jobs;
        jobs.reserve(snapshot.size());

        for (auto &artwork: snapshot) {
            jobs.emplace_back(std::make_shared<MetadataReadWriteTask>(artwork, MetadataReadWriteTask::Add));
        }

        m_MetadataIOWorker->submitItems(jobs);
        m_MetadataIOWorker->submitSeparator();
    }

    void MetadataIOService::searchArtworks(Suggestion::LocalLibraryQuery *query) {
        LOG_DEBUG << "#";
        Q_ASSERT(query != nullptr);
        if (m_IsStopped) { return; }
        auto jobItem = std::make_shared<MetadataSearchTask>(query);
        m_MetadataIOWorker->submitFirst(jobItem);
    }

    void MetadataIOService::workerFinished() {
        LOG_DEBUG << "#";
    }

    void MetadataIOService::onCacheSyncRequest() {
        LOG_DEBUG << "#";
        justChanged();
}

    void MetadataIOService::onReadyToImportFromStorage() {
        LOG_DEBUG << "#";
        if (m_IsStopped) { return; }
        // DO NOT remove this method to simplify worker call
        // service object exists in main thread and worker signal/slot
        // will be scheduled in worker thread instead of main
        m_MetadataIOWorker->importArtworksFromStorage();
    }

    void MetadataIOService::doOnTimer() {
        LOG_DEBUG << "#";
        if (m_IsStopped) { return; }
        m_MetadataIOWorker->submitSeparator();
    }
}

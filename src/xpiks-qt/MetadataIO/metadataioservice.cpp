/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2017 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "metadataioservice.h"
#include <QThread>
#include <QTimerEvent>
#include "metadataioworker.h"
#include "metadataiotask.h"
#include "../Commands/commandmanager.h"
#include "../Helpers/database.h"

#define SAVER_TIMER_TIMEOUT 3000
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

    void MetadataIOService::startService() {
        Q_ASSERT(m_MetadataIOWorker == nullptr);
        Helpers::DatabaseManager *dbManager = m_CommandManager->getDatabaseManager();
        QMLExtensions::ArtworksUpdateHub *updateHub = m_CommandManager->getArtworksUpdateHub();

        m_MetadataIOWorker = new MetadataIOWorker(dbManager, updateHub);

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

    void MetadataIOService::writeArtwork(Models::ArtworkMetadata *metadata) {
        Q_ASSERT(metadata != nullptr);
        if (m_IsStopped) { return; }
        LOG_DEBUG << "Saving" << metadata->getItemID();

        std::shared_ptr<MetadataIOTaskBase> jobItem(new MetadataReadWriteTask(metadata, MetadataReadWriteTask::Write));
        m_MetadataIOWorker->submitItem(jobItem);

        emit cacheSyncRequest();
    }

    quint32 MetadataIOService::readArtworks(const ArtworksSnapshot &snapshot) const {
        LOG_INFO << snapshot.size() << "artwork(s)";
        if (m_IsStopped) { return 0; }
        std::vector<std::shared_ptr<MetadataIOTaskBase> > jobs;
        jobs.reserve(snapshot.size());

        auto &items = snapshot.getRawData();

        for (auto &item: items) {
            Models::ArtworkMetadata *artwork = item->getArtworkMetadata();
            jobs.emplace_back(new MetadataReadWriteTask(artwork, MetadataReadWriteTask::Read));
        }

        MetadataIOWorker::batch_id_t batchID = m_MetadataIOWorker->submitItems(jobs);
        m_MetadataIOWorker->submitSeparator();
        LOG_INFO << "Batch ID is" << batchID;

        return batchID;
    }

    void MetadataIOService::writeArtworks(const WeakArtworksSnapshot &artworks) const {
        LOG_INFO << artworks.size() << "artwork(s)";
        if (m_IsStopped) { return; }
        std::vector<std::shared_ptr<MetadataIOTaskBase> > jobs;
        jobs.reserve(artworks.size());

        size_t size = artworks.size();
        for (size_t i = 0; i < size; ++i) {
            Models::ArtworkMetadata *artwork = artworks.at(i);
            jobs.emplace_back(new MetadataReadWriteTask(artwork, MetadataReadWriteTask::Write));
        }

        m_MetadataIOWorker->submitItems(jobs);
        m_MetadataIOWorker->submitSeparator();
    }

    void MetadataIOService::addArtworks(const WeakArtworksSnapshot &artworks) const {
        LOG_INFO << artworks.size() << "artwork(s)";
        if (m_IsStopped) { return; }
        std::vector<std::shared_ptr<MetadataIOTaskBase> > jobs;
        jobs.reserve(artworks.size());

        size_t size = artworks.size();
        for (size_t i = 0; i < size; ++i) {
            Models::ArtworkMetadata *artwork = artworks.at(i);
            jobs.emplace_back(new MetadataReadWriteTask(artwork, MetadataReadWriteTask::Add));
        }

        m_MetadataIOWorker->submitItems(jobs);
        m_MetadataIOWorker->submitSeparator();
    }

    void MetadataIOService::searchArtworks(Suggestion::LocalLibraryQuery *query) {
        LOG_DEBUG << "#";
        Q_ASSERT(query != nullptr);
        if (m_IsStopped) { return; }
        std::shared_ptr<MetadataSearchTask> jobItem(new MetadataSearchTask(query));
        m_MetadataIOWorker->submitFirst(jobItem);
    }

    void MetadataIOService::workerFinished() {
        LOG_INFO << "#";
    }

    void MetadataIOService::onCacheSyncRequest() {
        LOG_DEBUG << "#";
        justChanged();
}

    void MetadataIOService::onReadyToImportFromStorage() {
        LOG_DEBUG << "#";
        if (m_IsStopped) { return; }
        m_MetadataIOWorker->importArtworksFromStorage();
    }

    void MetadataIOService::doOnTimer() {
        m_MetadataIOWorker->submitSeparator();
    }
}

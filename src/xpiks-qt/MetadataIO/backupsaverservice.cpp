/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2017 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "backupsaverservice.h"
#include "backupsaverworker.h"
#include "../Models/artworkmetadata.h"
#include "saverworkerjobitem.h"
#include "../Common/defines.h"

namespace MetadataIO {
    BackupSaverService::BackupSaverService():
        QObject()
    {
        m_BackupWorker = new BackupSaverWorker();
    }

    void BackupSaverService::startSaving() {
        Q_ASSERT(!m_BackupWorker->isRunning());
        LOG_DEBUG << "#";

        QThread *thread = new QThread();
        m_BackupWorker->moveToThread(thread);

        QObject::connect(thread, SIGNAL(started()), m_BackupWorker, SLOT(process()));
        QObject::connect(m_BackupWorker, SIGNAL(stopped()), thread, SLOT(quit()));

        QObject::connect(m_BackupWorker, SIGNAL(stopped()), m_BackupWorker, SLOT(deleteLater()));
        QObject::connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));

        QObject::connect(this, SIGNAL(cancelSaving()),
                         m_BackupWorker, SLOT(cancel()));

        QObject::connect(m_BackupWorker, SIGNAL(stopped()),
                         this, SLOT(workerFinished()));

        thread->start();
    }

    void BackupSaverService::saveArtwork(Models::ArtworkMetadata *metadata) const {
        LOG_INFO << metadata->getFilepath();
        std::shared_ptr<SaverWorkerJobItem> jobItem(new SaverWorkerJobItem(metadata));
        m_BackupWorker->submitItem(jobItem);
    }

    void BackupSaverService::saveArtworks(const QVector<Models::ArtworkMetadata *> &artworks) const {
        LOG_INFO << artworks.size() << "artwork(s)";
        std::vector<std::shared_ptr<SaverWorkerJobItem> > jobs;
        jobs.reserve(artworks.length());

        int size = artworks.size();
        for (int i = 0; i < size; ++i) {
            jobs.emplace_back(new SaverWorkerJobItem(artworks.at(i)));
        }

        m_BackupWorker->submitItems(jobs);
    }

    void BackupSaverService::workerFinished() {
        LOG_INFO << "#";
    }

    void BackupSaverService::stopSaving() {
        LOG_DEBUG << "stopping...";
        m_BackupWorker->stopWorking();
    }
}


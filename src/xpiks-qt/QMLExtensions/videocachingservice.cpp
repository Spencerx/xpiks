/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "videocachingservice.h"
#include <QThread>
#include <vector>
#include <memory>
#include "videocachingworker.h"
#include <Artworks/videoartwork.h>
#include "../QMLExtensions/videocacherequest.h"
#include "../Models/switchermodel.h"
#include "../Common/logging.h"

namespace QMLExtensions {
    VideoCachingService::VideoCachingService(Common::ISystemEnvironment &environment, Storage::IDatabaseManager *dbManager, QObject *parent) :
        QObject(parent),
        Common::BaseEntity(),
        m_Environment(environment),
        m_DatabaseManager(dbManager),
        m_CachingWorker(nullptr),
        m_IsCancelled(false)
    {
        Q_ASSERT(dbManager != nullptr);
    }

    void VideoCachingService::startService() {
        m_CachingWorker = new VideoCachingWorker(m_Environment, m_DatabaseManager);
        m_CachingWorker->setCommandManager(m_CommandManager);

        QThread *thread = new QThread();
        m_CachingWorker->moveToThread(thread);

        QObject::connect(thread, SIGNAL(started()), m_CachingWorker, SLOT(process()));
        QObject::connect(m_CachingWorker, SIGNAL(stopped()), thread, SLOT(quit()));

        QObject::connect(m_CachingWorker, SIGNAL(stopped()), m_CachingWorker, SLOT(deleteLater()));
        QObject::connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));

        LOG_DEBUG << "starting low priority thread...";
        thread->start(QThread::LowPriority);
    }

    void VideoCachingService::stopService() {
        LOG_DEBUG << "#";

        if (m_CachingWorker != NULL) {
            m_IsCancelled = true;
            m_CachingWorker->stopWorking();
        } else {
            LOG_WARNING << "Caching Worker was NULL";
        }
    }

    void VideoCachingService::generateThumbnails(const Artworks::ArtworksSnapshot &snapshot) {
        Q_ASSERT(m_CachingWorker != nullptr);
        LOG_INFO << snapshot.size() << "artworks";

#ifndef INTEGRATION_TESTS
        Models::SwitcherModel *switcher = m_CommandManager->getSwitcherModel();
        const bool goodQualityAllowed = switcher->getGoodQualityVideoPreviews();
#else
        const bool goodQualityAllowed = false;
#endif
        LOG_DEBUG << (goodQualityAllowed ? "Good" : "Quick") << "quality allowed";

        const size_t size = snapshot.size();
        std::vector<std::shared_ptr<VideoCacheRequest> > requests;
        requests.reserve(size);

        for (size_t i = 0; i < size; i++) {
            auto *artwork = snapshot.get(i);
            Artworks::VideoArtwork *videoArtwork = dynamic_cast<Artworks::VideoArtwork *>(artwork);
            if (videoArtwork != nullptr) {
                const bool quickThumbnail = true, dontRecache = false;
                requests.emplace_back(new VideoCacheRequest(videoArtwork,
                                                            dontRecache,
                                                            quickThumbnail,
                                                            goodQualityAllowed));
            }
        }

        m_CachingWorker->submitItems(requests);
        m_CachingWorker->submitSeparator();
    }

    void VideoCachingService::generateThumbnail(Artworks::VideoArtwork *videoArtwork) {
        Q_ASSERT(videoArtwork != nullptr);
        if (videoArtwork == nullptr) { return; }
        LOG_DEBUG << "#" << videoArtwork->getItemID();

#ifndef INTEGRATION_TESTS
        Models::SwitcherModel *switcher = m_CommandManager->getSwitcherModel();
        const bool goodQualityAllowed = switcher->getGoodQualityVideoPreviews();
#else
        const bool goodQualityAllowed = false;
#endif
        const bool quickThumbnail = true, dontRecache = false;

        std::shared_ptr<VideoCacheRequest> request(new VideoCacheRequest(videoArtwork,
                                                                         dontRecache,
                                                                         quickThumbnail,
                                                                         goodQualityAllowed));
        m_CachingWorker->submitItem(request);
    }

    void VideoCachingService::waitWorkerIdle() {
        LOG_DEBUG << "#";
        Q_ASSERT(m_CachingWorker != nullptr);
        m_CachingWorker->waitIdle();
    }
}

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

#include <cstddef>
#include <memory>
#include <vector>

#include <QThread>
#include <QtDebug>
#include <QtGlobal>

#include "Artworks/artworkssnapshot.h"
#include "Artworks/videoartwork.h"
#include "Common/logging.h"
#include "Models/switchermodel.h"
#include "QMLExtensions/videocacherequest.h"
#include "QMLExtensions/videocachingworker.h"

namespace QMLExtensions {
    VideoCachingService::VideoCachingService(Common::ISystemEnvironment &environment,
                                             Models::SwitcherModel &switcherModel,
                                             QObject *parent) :
        QObject(parent),
        m_Environment(environment),
        m_SwitcherModel(switcherModel),
        m_CachingWorker(nullptr),
        m_IsCancelled(false)
    {
    }

    void VideoCachingService::startService(ImageCachingService &imageCachingService,
                                           Services::ArtworksUpdateHub &updateHub,
                                           MetadataIO::MetadataIOService &metadataIOService,
                                           Storage::IDatabaseManager &dbManager) {
        m_CachingWorker = new VideoCachingWorker(m_Environment,
                                                 dbManager,
                                                 imageCachingService,
                                                 updateHub,
                                                 metadataIOService);

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

        if (m_CachingWorker != nullptr) {
            m_IsCancelled = true;
            m_CachingWorker->stopWorking();
        } else {
            LOG_WARNING << "Caching Worker was nullptr";
        }
    }

    void VideoCachingService::generateThumbnails(const Artworks::ArtworksSnapshot &snapshot) {
        Q_ASSERT(m_CachingWorker != nullptr);
        LOG_INFO << snapshot.size() << "artworks";

#ifndef INTEGRATION_TESTS
        const bool goodQualityAllowed = m_SwitcherModel.getGoodQualityVideoPreviews();
#else
        const bool goodQualityAllowed = false;
#endif
        LOG_DEBUG << (goodQualityAllowed ? "Good" : "Quick") << "quality allowed";

        const size_t size = snapshot.size();
        std::vector<std::shared_ptr<VideoCacheRequest> > requests;
        requests.reserve(size);

        for (auto &artwork: snapshot) {
            auto videoArtwork = std::dynamic_pointer_cast<Artworks::VideoArtwork>(artwork);
            if (videoArtwork != nullptr) {
                const bool quickThumbnail = true, dontRecache = false;
                requests.emplace_back(std::make_shared<VideoCacheRequest>(videoArtwork,
                                                                          dontRecache,
                                                                          quickThumbnail,
                                                                          goodQualityAllowed));
            }
        }

        m_CachingWorker->submitItems(requests);
        m_CachingWorker->submitSeparator();
    }

    void VideoCachingService::generateThumbnail(std::shared_ptr<Artworks::VideoArtwork> const &videoArtwork) {
        Q_ASSERT(videoArtwork != nullptr);
        if (videoArtwork == nullptr) { return; }
        LOG_DEBUG << "#" << videoArtwork->getItemID();

#ifndef INTEGRATION_TESTS
        const bool goodQualityAllowed = m_SwitcherModel.getGoodQualityVideoPreviews();
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

/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "imagecachingservice.h"
#include <QThread>
#include <QScreen>
#include "imagecachingworker.h"
#include "imagecacherequest.h"
#include "../Models/artworkmetadata.h"
#include "../Models/imageartwork.h"
#include "../Helpers/asynccoordinator.h"
#include "../Commands/commandmanager.h"
#include "../MetadataIO/artworkssnapshot.h"

namespace QMLExtensions {
    ImageCachingService::ImageCachingService(Common::ISystemEnvironment &environment, QObject *parent) :
        QObject(parent),
        Common::BaseEntity(),
        m_Environment(environment),
        m_CachingWorker(NULL),
        m_IsCancelled(false),
        m_Scale(1.0)
    {
        updateDefaultSize();
    }

    void ImageCachingService::startService(const std::shared_ptr<Common::ServiceStartParams> &params) {
        auto coordinatorParams = std::dynamic_pointer_cast<Helpers::AsyncCoordinatorStartParams>(params);

        Helpers::AsyncCoordinator *coordinator = nullptr;
        if (coordinatorParams) { coordinator = coordinatorParams->m_Coordinator; }

        Helpers::AsyncCoordinatorLocker locker(coordinator);
        Q_UNUSED(locker);

        auto *dbManager = m_CommandManager->getDatabaseManager();
        m_CachingWorker = new ImageCachingWorker(m_Environment, coordinator, dbManager);

        QThread *thread = new QThread();
        m_CachingWorker->moveToThread(thread);

        QObject::connect(thread, &QThread::started, m_CachingWorker, &ImageCachingWorker::process);
        QObject::connect(m_CachingWorker, &ImageCachingWorker::stopped, thread, &QThread::quit);

        QObject::connect(m_CachingWorker, &ImageCachingWorker::stopped, m_CachingWorker, &ImageCachingWorker::deleteLater);
        QObject::connect(thread, &QThread::finished, thread, &QThread::deleteLater);

        LOG_DEBUG << "starting low priority thread...";
        thread->start(QThread::LowPriority);
    }

    void ImageCachingService::stopService() {
        LOG_DEBUG << "#";

        if (m_CachingWorker != NULL) {
            m_IsCancelled = true;
            m_CachingWorker->stopWorking();
        } else {
            LOG_WARNING << "Caching Worker was NULL";
        }
    }

    void ImageCachingService::upgradeCacheStorage() {
        LOG_DEBUG << "#";

        if ((m_CachingWorker != NULL) && !m_IsCancelled) {
            bool migrated = m_CachingWorker->upgradeCacheStorage();
            if (migrated) {
                m_CachingWorker->submitSeparator();
            }
        }
    }

    void ImageCachingService::setScale(qreal scale) {
        LOG_INFO << scale;
        if ((0.99f < scale) && (scale < 5.0f)) {
            m_Scale = scale;
            if (m_CachingWorker != nullptr) {
                m_CachingWorker->setScale(scale);
            }
            LOG_INFO << "Scale is now" << m_Scale;
            updateDefaultSize();
        }
    }

    void ImageCachingService::cacheImage(const QString &key, const QSize &requestedSize, bool recache) {
        if (m_IsCancelled) { return; }

        Q_ASSERT(m_CachingWorker != NULL);
        std::shared_ptr<ImageCacheRequest> request(new ImageCacheRequest(key, requestedSize, recache));
        m_CachingWorker->submitFirst(request);
    }

    void ImageCachingService::cacheImage(const QString &key) {
        this->cacheImage(key, QSize(DEFAULT_THUMB_WIDTH * m_Scale, DEFAULT_THUMB_HEIGHT * m_Scale));
    }

    void ImageCachingService::generatePreviews(const MetadataIO::ArtworksSnapshot &snapshot) {
        if (m_IsCancelled) { return; }

        Q_ASSERT(m_CachingWorker != NULL);
        LOG_INFO << "generating for" << snapshot.size() << "items";

        std::vector<std::shared_ptr<ImageCacheRequest> > requests;
        const size_t size = snapshot.size();
        requests.reserve(size);
        const bool recache = false;

        updateDefaultSize();

        for (size_t i = 0; i < size; i++) {
            auto *artwork = snapshot.get(i);
            Models::ImageArtwork *imageArtwork = dynamic_cast<Models::ImageArtwork*>(artwork);
            if (imageArtwork != nullptr) {
                requests.emplace_back(new ImageCacheRequest(artwork->getThumbnailPath(),
                                                            m_DefaultSize,
                                                            recache));
            }
        }

        m_CachingWorker->submitItems(requests);
        m_CachingWorker->submitSeparator();
    }

    bool ImageCachingService::tryGetCachedImage(const QString &key, const QSize &requestedSize,
                                                QString &cached, bool &needsUpdate) {
        if (!m_IsCancelled && m_CachingWorker != NULL) {
            return m_CachingWorker->tryGetCachedImage(key, requestedSize, cached, needsUpdate);
        } else {
            return false;
        }
    }

    void ImageCachingService::updateDefaultSize() {
        m_DefaultSize.setHeight(DEFAULT_THUMB_HEIGHT * m_Scale);
        m_DefaultSize.setWidth(DEFAULT_THUMB_WIDTH * m_Scale);

        LOG_DEBUG << "Default size is" << m_DefaultSize.height() << "x" << m_DefaultSize.width();
    }

    void ImageCachingService::screenChangedHandler(QScreen *screen) {
        LOG_DEBUG << "#";
        if (screen != nullptr) {
            setScale(screen->devicePixelRatio());
        }
    }

    void ImageCachingService::dpiChanged(qreal someDPI) {
        LOG_DEBUG << "#";
        Q_UNUSED(someDPI);
        QScreen *screen = qobject_cast<QScreen*>(sender());
        if (screen != nullptr) {
            setScale(screen->devicePixelRatio());
        }
    }
}

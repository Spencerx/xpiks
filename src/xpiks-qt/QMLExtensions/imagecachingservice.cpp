/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2017 Taras Kushnir <kushnirTV@gmail.com>
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

namespace QMLExtensions {
    ImageCachingService::ImageCachingService(QObject *parent) :
        QObject(parent),
        m_CachingWorker(NULL),
        m_IsCancelled(false),
        m_Scale(1.0)
    {
    }

    void ImageCachingService::startService() {
        m_CachingWorker = new ImageCachingWorker();

        QThread *thread = new QThread();
        m_CachingWorker->moveToThread(thread);

        QObject::connect(thread, SIGNAL(started()), m_CachingWorker, SLOT(process()));
        QObject::connect(m_CachingWorker, SIGNAL(stopped()), thread, SLOT(quit()));

        QObject::connect(m_CachingWorker, SIGNAL(stopped()), m_CachingWorker, SLOT(deleteLater()));
        QObject::connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));

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

    void ImageCachingService::setScale(qreal scale) {
        LOG_INFO << scale;
        if ((0.99f < scale) && (scale < 5.0f)) {
            m_Scale = scale;
            if (m_CachingWorker != nullptr) {
                m_CachingWorker->setScale(scale);
            }
            LOG_INFO << "Scale is now" << m_Scale;
        }
    }

    void ImageCachingService::cacheImage(const QString &key, const QSize &requestedSize, bool recache) {
        if (m_IsCancelled) { return; }

        Q_ASSERT(m_CachingWorker != NULL);
        std::shared_ptr<ImageCacheRequest> request(new ImageCacheRequest(key, requestedSize, recache));
        m_CachingWorker->submitFirst(request);
    }

    void ImageCachingService::generatePreviews(const QVector<Models::ArtworkMetadata *> &items) {
        if (m_IsCancelled) { return; }

        Q_ASSERT(m_CachingWorker != NULL);
        LOG_INFO << "generating for" << items.size() << "items";

        std::vector<std::shared_ptr<ImageCacheRequest> > requests;
        int size = items.size();
        requests.reserve(size);
        const bool recache = false;

        for (int i = 0; i < size; ++i) {
            bool withDelay = i % 2;
            Models::ArtworkMetadata *artwork = items.at(i);
            requests.emplace_back(new ImageCacheRequest(artwork->getFilepath(),
                                                        QSize(DEFAULT_THUMB_WIDTH * m_Scale, DEFAULT_THUMB_HEIGHT * m_Scale),
                                                        recache,
                                                        withDelay));
        }

        std::vector<std::shared_ptr<ImageCacheRequest> > knownRequests;
        std::vector<std::shared_ptr<ImageCacheRequest> > unknownRequests;
        m_CachingWorker->splitToCachedAndNot(requests, unknownRequests, knownRequests);

        m_CachingWorker->submitFirst(unknownRequests);
        m_CachingWorker->submitItems(knownRequests);
    }

    bool ImageCachingService::tryGetCachedImage(const QString &key, const QSize &requestedSize,
                                                QString &cached, bool &needsUpdate) {
        if (!m_IsCancelled && m_CachingWorker != NULL) {
            return m_CachingWorker->tryGetCachedImage(key, requestedSize, cached, needsUpdate);
        } else {
            return false;
        }
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

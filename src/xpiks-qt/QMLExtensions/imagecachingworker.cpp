/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "imagecachingworker.h"
#include <QDir>
#include <QFile>
#include <QImage>
#include <QString>
#include <QFileInfo>
#include <QByteArray>
#include <QDataStream>
#include <QReadLocker>
#include <QWriteLocker>
#include <QCryptographicHash>
#include "../Common/defines.h"
#include "../Helpers/constants.h"
#include "imagecacherequest.h"
#include "../Helpers/asynccoordinator.h"
#include "dbimagecacheindex.h"

#define IMAGE_CACHING_WORKER_SLEEP_DELAY 500
#define IMAGES_INDEX_BACKUP_STEP 50
#define PREVIEW_JPG_QUALITY 70

namespace QMLExtensions {
    QString getImagePathHash(const QString &path) {
        return QString::fromLatin1(QCryptographicHash::hash(path.toUtf8(), QCryptographicHash::Sha256).toHex());
    }

    ImageCachingWorker::ImageCachingWorker(Common::ISystemEnvironment &environment,
                                           Helpers::AsyncCoordinator *initCoordinator,
                                           Helpers::DatabaseManager *dbManager,
                                           QObject *parent):
        QObject(parent),
        ItemProcessingWorker(2),
        m_Environment(environment),
        m_InitCoordinator(initCoordinator),
        m_ProcessedItemsCount(0),
        m_Cache(dbManager),
        m_Scale(1.0)
    {
    }

    bool ImageCachingWorker::initWorker() {
        LOG_DEBUG << "#";

        Helpers::AsyncCoordinatorUnlocker unlocker(m_InitCoordinator);
        Q_UNUSED(unlocker);

        m_ProcessedItemsCount = 0;

        m_Environment.ensureDirExists(Constants::IMAGES_CACHE_DIR);
        m_ImagesCacheDir = m_Environment.dirpath(Constants::IMAGES_CACHE_DIR);
        LOG_INFO << "Using" << m_ImagesCacheDir << "for images cache";

        m_Cache.initialize();

        return true;
    }

    void ImageCachingWorker::processOneItemEx(std::shared_ptr<ImageCacheRequest> &item, batch_id_t batchID, Common::flag_t flags) {
        Q_UNUSED(batchID);

        if (getIsSeparatorFlag(flags)) {
            saveIndex();
        } else {
            ItemProcessingWorker::processOneItemEx(item, batchID, flags);

            if (getWithDelayFlag(flags)) {
                // force context switch for more imporant tasks
                QThread::msleep(IMAGE_CACHING_WORKER_SLEEP_DELAY);
            }
        }
    }

    void ImageCachingWorker::processOneItem(std::shared_ptr<ImageCacheRequest> &item) {
        if (isProcessed(item)) {
            LOG_FOR_DEBUG << item->getFilepath() << "is processed";
            return;
        }

        const QString &originalPath = item->getFilepath();
        QSize requestedSize = item->getRequestedSize();

        LOG_INFO << (item->getNeedRecache() ? "Recaching" : "Caching") << originalPath << "with size" << requestedSize;

        if (!requestedSize.isValid()) {
            LOG_WARNING << "Invalid requestedSize for" << originalPath;
            requestedSize.setHeight(DEFAULT_THUMB_HEIGHT * m_Scale);
            requestedSize.setWidth(DEFAULT_THUMB_WIDTH * m_Scale);
        }

        const bool isInResources = originalPath.startsWith(":/");

        QImage img;
        bool isLoaded = img.load(originalPath);
        if (!isLoaded || img.isNull()) {
            LOG_WARNING << "Image" << originalPath << "is null image";
            return;
        }

        QImage resizedImage = img.scaled(requestedSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);

        QFileInfo fi(originalPath);
        const QString suffix = isInResources ? "jpg" : fi.suffix();
        QString pathHash = getImagePathHash(originalPath) + "." + suffix;
        QString cachedFilepath = QDir::cleanPath(m_ImagesCacheDir + QDir::separator() + pathHash);

        if (resizedImage.save(cachedFilepath, nullptr, PREVIEW_JPG_QUALITY)) {
            CachedImage cachedImage;
            cachedImage.m_Filename = pathHash;
            cachedImage.m_LastModified = isInResources ? QDateTime::currentDateTime() : fi.lastModified();
            cachedImage.m_Size = requestedSize;

            m_Cache.update(originalPath, cachedImage);

            m_ProcessedItemsCount++;
        } else {
            LOG_WARNING << "Failed to save image. Path:" << cachedFilepath << "size" << requestedSize;
        }

        if (m_ProcessedItemsCount % IMAGES_INDEX_BACKUP_STEP == 0) {
            saveIndex();
        }
    }

    void ImageCachingWorker::workerStopped() {
        LOG_DEBUG << "#";
        m_Cache.finalize();
        emit stopped();
    }

    bool ImageCachingWorker::tryGetCachedImage(const QString &key, const QSize &requestedSize,
                                               QString &cachedPath, bool &needsUpdate) {
        bool found = false;
        CachedImage cachedImage;

        if (m_Cache.tryGet(key, cachedImage)) {
            QString cachedValue = QDir::cleanPath(m_ImagesCacheDir + QDir::separator() + cachedImage.m_Filename);

            QFileInfo fi(cachedValue);

            if (fi.exists()) {
                cachedImage.m_RequestsServed++;
                cachedPath = cachedValue;
                const bool isInResources = key.startsWith(":/");
                const bool isOutdated = (!isInResources) && (QFileInfo(key).lastModified() > cachedImage.m_LastModified);
                needsUpdate = isOutdated || (cachedImage.m_Size != requestedSize);

                found = true;
            }
        }

        return found;
    }

    bool ImageCachingWorker::upgradeCacheStorage() {
        bool migrated = false;
        QString indexFilepath = m_Environment.filepath(Constants::IMAGES_CACHE_INDEX);
        LOG_INFO << "Trying to load old cache index from" << indexFilepath;

        QHash<QString, CachedImage> oldCache;

        QFile file(indexFilepath);
        if (file.open(QIODevice::ReadOnly)) {
            QDataStream in(&file);   // read the data
            in >> oldCache;
            file.close();

            LOG_INFO << "Read" << oldCache.size() << "items from the old cache index";

            m_Cache.importCache(oldCache);
            migrated = true;

            if (file.rename(indexFilepath + ".backup")) {
                LOG_INFO << "Old cache index has been discarded";
            } else {
                LOG_WARNING << "Failed to discard old cache index";
            }
        } else {
            LOG_INFO << "Cannot open old cache index";
        }

        return migrated;
    }

    void ImageCachingWorker::saveIndex() {
        m_Cache.sync();
    }

    bool ImageCachingWorker::isProcessed(std::shared_ptr<ImageCacheRequest> &item) {
        if (item->getNeedRecache()) { return false; }

        const QString &originalPath = item->getFilepath();
        const QSize &requestedSize = item->getRequestedSize();

        bool isAlreadyProcessed = false;

        QString cachedPath;
        bool needsUpdate = false;
        if (this->tryGetCachedImage(originalPath, requestedSize, cachedPath, needsUpdate)) {
            isAlreadyProcessed = !needsUpdate;
        }

        return isAlreadyProcessed;
    }
}

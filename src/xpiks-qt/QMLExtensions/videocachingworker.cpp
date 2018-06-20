/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "videocachingworker.h"
#include <QDir>
#include <QImage>
#include <QThread>
#include <QCryptographicHash>
#include <vector>
#include <cstdint>
#include "../Helpers/constants.h"
#include "imagecachingservice.h"
#include "../Services/artworksupdatehub.h"
#include "../MetadataIO/metadataioservice.h"
#include "../Models/artitemsmodel.h"
#include "../Commands/commandmanager.h"
#include <thumbnailcreator.h>

#define VIDEO_WORKER_SLEEP_DELAY 500
#define VIDEO_INDEX_BACKUP_STEP 50
#define THUMBNAIL_JPG_QUALITY 80

namespace QMLExtensions {
    QString getVideoPathHash(const QString &path, bool isQuickThumbnail) {
        QString hash = QString::fromLatin1(QCryptographicHash::hash(path.toUtf8(), QCryptographicHash::Sha256).toHex());
        // TODO: in the video cache cleanup remove all with same hash without _s
        if (!isQuickThumbnail) { hash.append(QChar('s')); }
        return hash;
    }

    VideoCachingWorker::VideoCachingWorker(Common::ISystemEnvironment &environment,
                                           Storage::IDatabaseManager *dbManager,
                                           QObject *parent) :
        QObject(parent),
        ItemProcessingWorker(2),
        m_Environment(environment),
        m_ProcessedItemsCount(0),
        m_Scale(1.0),
        m_Cache(dbManager)
    {
        m_RolesToUpdate << Models::ArtItemsModel::ArtworkThumbnailRole;
    }

    bool VideoCachingWorker::initWorker() {
        LOG_DEBUG << "#";

        m_ProcessedItemsCount = 0;

        m_Environment.ensureDirExists(Constants::VIDEO_CACHE_DIR);
        m_VideosCacheDir = m_Environment.path({Constants::VIDEO_CACHE_DIR});
        LOG_INFO << "Using" << m_VideosCacheDir << "for videos cache";

        m_Cache.initialize();

        return true;
    }

    void VideoCachingWorker::processOneItemEx(std::shared_ptr<VideoCacheRequest> &item, batch_id_t batchID, Common::flag_t flags) {
        if (getIsSeparatorFlag(flags)) {
            saveIndex();
        } else {
            ItemProcessingWorker::processOneItemEx(item, batchID, flags);

            if (getIsMilestone(flags)) {
                // force context switch for more imporant tasks
                QThread::msleep(VIDEO_WORKER_SLEEP_DELAY);
            }
        }
    }

    void VideoCachingWorker::processOneItem(std::shared_ptr<VideoCacheRequest> &item) {
        LOG_DEBUG << "Processing #" << item->getArtworkID();
        if (checkLockedIO(item)) { return; }
        if (checkProcessed(item)) { return; }

        const QString &originalPath = item->getFilepath();
        const bool isQuickThumbnail = item->getIsQuickThumbnail();
        LOG_INFO << (item->getNeedRecache() ? "Recaching" : "Caching") << originalPath;
        LOG_INFO << (isQuickThumbnail ? "Quick thumbnail" : "Good thumbnail") << "requested";

        std::vector<uint8_t> buffer;
        int width = 0, height = 0;
        bool needsRefresh = false, success = false;

        if (createThumbnail(item, buffer, width, height)) {
            QString thumbnailPath;
            QImage image((unsigned char*)&buffer[0], width, height, QImage::Format_RGB888);

            if (saveThumbnail(image, originalPath, isQuickThumbnail, thumbnailPath)) {
                cacheImage(thumbnailPath);
                applyThumbnail(item, thumbnailPath, true);

                if (m_ProcessedItemsCount % VIDEO_INDEX_BACKUP_STEP == 0) {
                    saveIndex();
                }

                if (isQuickThumbnail && item->getGoodQualityAllowed()) {
                    LOG_INTEGR_TESTS_OR_DEBUG << "Regenerating good quality thumb for" << originalPath;
                    item->setGoodQualityRequest();
                    needsRefresh = true;
                }

                success = true;
            }
        }

        if (!success && !item->isRepeated()) {
            item->setRepeatRequest();
            needsRefresh = true;
        }

        if (needsRefresh) {
            this->submitItem(item);
        }
    }

    bool VideoCachingWorker::createThumbnail(std::shared_ptr<VideoCacheRequest> &item, std::vector<uint8_t> &buffer, int &width, int &height) {
        bool thumbnailCreated = false;

        const QString &originalPath = item->getFilepath();
        const QString filepath = QDir::toNativeSeparators(originalPath);
#ifdef Q_OS_WIN
        libthmbnlr::ThumbnailCreator thumbnailCreator(filepath.toStdWString());
#else
        libthmbnlr::ThumbnailCreator thumbnailCreator(filepath.toStdString());
#endif
        try {
            const libthmbnlr::ThumbnailCreator::CreationOption option = item->getIsQuickThumbnail() ? libthmbnlr::ThumbnailCreator::Quick : libthmbnlr::ThumbnailCreator::GoodQuality;
            thumbnailCreator.setCreationOption(option);
            thumbnailCreator.setSeekPercentage(50);
            thumbnailCreated = thumbnailCreator.createThumbnail(buffer, width, height);
            LOG_INTEGR_TESTS_OR_DEBUG << "Thumb generated for" << originalPath;
            if (thumbnailCreated) {
                item->setVideoMetadata(thumbnailCreator.getMetadata());
            } else {
                LOG_WARNING << "Failed to create thumbnail";
            }
        } catch (...) {
            LOG_WARNING << "Unknown exception while creating thumbnail";
        }

        return thumbnailCreated;
    }

    void VideoCachingWorker::workerStopped() {
        LOG_DEBUG << "#";
        m_Cache.finalize();
        emit stopped();
    }

    bool VideoCachingWorker::tryGetVideoThumbnail(const QString &key, QString &cachedPath, bool &needsUpdate) {
        bool found = false;
        CachedVideo cachedVideo;

        if (m_Cache.tryGet(key, cachedVideo)) {
            QString cachedValue = QDir::cleanPath(m_VideosCacheDir + QChar('/') + cachedVideo.m_Filename);

            QFileInfo fi(cachedValue);

            if (fi.exists()) {
                cachedVideo.m_RequestsServed++;
                cachedPath = cachedValue;
                needsUpdate = QFileInfo(key).lastModified() > cachedVideo.m_LastModified;

                found = true;
            }
        }

        return found;
    }

    bool VideoCachingWorker::saveThumbnail(QImage &image, const QString &originalPath, bool isQuickThumbnail, QString &thumbnailPath) {
        bool success = false;

        QFileInfo fi(originalPath);
        QString pathHash = getVideoPathHash(originalPath, isQuickThumbnail) + ".jpg";
        QString cachedFilepath = QDir::cleanPath(m_VideosCacheDir + QChar('/') + pathHash);

        if (image.save(cachedFilepath, "JPG", THUMBNAIL_JPG_QUALITY)) {
            CachedVideo cachedVideo;
            cachedVideo.m_Filename = pathHash;
            cachedVideo.m_LastModified = fi.lastModified();
            cachedVideo.m_IsQuickThumbnail = isQuickThumbnail;

            m_Cache.update(originalPath, cachedVideo);

            m_ProcessedItemsCount++;
            thumbnailPath = cachedFilepath;
            success = true;
        } else {
            LOG_WARNING << "Failed to save thumbnail. Path:" << cachedFilepath;
        }

        return success;
    }

    void VideoCachingWorker::cacheImage(const QString &thumbnailPath) {
        auto *imageCachingService = m_CommandManager->getImageCachingService();
        imageCachingService->cacheImage(thumbnailPath);
    }

    void VideoCachingWorker::applyThumbnail(std::shared_ptr<VideoCacheRequest> &item, const QString &thumbnailPath, bool recacheArtwork) {
        LOG_INFO << "#" << item->getArtworkID() << thumbnailPath;
        item->setThumbnailPath(thumbnailPath);

        auto *updateHub = m_CommandManager->getArtworksUpdateHub();
        updateHub->updateArtwork(item->getArtworkID(), item->getLastKnownIndex(), m_RolesToUpdate);

        if (recacheArtwork) {
            // write video metadata set to the artwork
            auto *metadataIOService = m_CommandManager->getMetadataIOService();
            metadataIOService->writeArtwork(item->getArtwork());
        }
    }

    void VideoCachingWorker::saveIndex() {
        LOG_DEBUG << "#";
        m_Cache.sync();
    }

    bool VideoCachingWorker::checkLockedIO(std::shared_ptr<VideoCacheRequest> &item) {
        bool isLocked = false;

        Artworks::VideoArtwork *video = item->getArtwork();
        Q_ASSERT(video != nullptr);
        if (video->isLockedIO()) {
            LOG_DEBUG << "video is locked for IO";
            this->submitItem(item);
            isLocked = true;
        }

        return isLocked;
    }

    bool VideoCachingWorker::checkProcessed(std::shared_ptr<VideoCacheRequest> &item) {
        if (item->getNeedRecache()) { return false; }
        if (item->isRepeated()) { return false; }

        const QString &originalPath = item->getFilepath();
        bool isAlreadyProcessed = false;

        QString cachedPath;
        bool needsUpdate = false;
        if (this->tryGetVideoThumbnail(originalPath, cachedPath, needsUpdate)) {
            isAlreadyProcessed = !needsUpdate;

            if (item->getThumbnailPath() != cachedPath) {
                LOG_DEBUG << "Updating outdated thumbnail of artwork #" << item->getArtworkID();
                applyThumbnail(item, cachedPath, false);
            }
        }

        return isAlreadyProcessed;
    }
}

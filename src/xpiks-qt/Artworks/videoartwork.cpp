/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "videoartwork.h"

#include <QFileInfo>
#include <QMutexLocker>
#include <QtDebug>

#include <vendors/libthmbnlr/videofilemetadata.h>

#include "Artworks/artworkmetadata.h"
#include "Common/logging.h"
#include "Common/types.h"
#include "MetadataIO/cachedartwork.h"
#include "MetadataIO/originalmetadata.h"

namespace Artworks {
    VideoArtwork::VideoArtwork(const QString &filepath, Common::ID_t ID, qint64 directoryID):
        ArtworkMetadata(filepath, ID, directoryID),
        m_VideoFlags(0),
        m_Duration(0.0),
        m_BitRate(0),
        m_FrameRate(0.0)
    {
        // this is a safer way than generating empty images and
        // handling empty thumbnail paths along everywhere
        m_ThumbnailPath = ":/Graphics/video-default-thumbnail.png";
    }

    void VideoArtwork::setThumbnailPath(const QString &filepath) {
        LOG_INFO << "#" << getItemID() << filepath;
        Q_ASSERT(!filepath.isEmpty());

        QMutexLocker locker(&m_ThumbnailLock);
        Q_UNUSED(locker);

        if (QFileInfo(filepath).exists()) {
            m_ThumbnailPath = filepath;
            setThumbnailGeneratedFlag(true);
            emit thumbnailUpdated();
        } else {
            LOG_WARNING << "Wrong thumbnail filepath:" << filepath;
        }
    }

    void VideoArtwork::setVideoMetadata(const libthmbnlr::VideoFileMetadata &metadata) {
        LOG_DEBUG << "#";

        m_CodecName = QString::fromStdString(metadata.m_CodecName);
        m_BitRate = metadata.m_BitRate;
        m_Duration = metadata.m_Duration;
        m_FrameRate = metadata.m_FrameRate;
    }

    void VideoArtwork::initializeThumbnailPath(const QString &filepath) {
        if (filepath.isEmpty()) { return; }

        QMutexLocker locker(&m_ThumbnailLock);
        Q_UNUSED(locker);

        if (getThumbnailGeneratedFlag()) { return; }

        if (QFileInfo(filepath).exists()) {
            LOG_INFO << "#" << getItemID() << filepath;
            m_ThumbnailPath = filepath;
        } else {
            LOG_WARNING << "Wrong thumbnail filepath:" << filepath;
        }
    }

    bool VideoArtwork::initFromOriginUnsafe(const MetadataIO::OriginalMetadata &originalMetadata) {
        m_FrameRate = originalMetadata.m_VideoFrameRate;
        m_BitRate = originalMetadata.m_VideoBitRate;
        m_Duration = originalMetadata.m_VideoDuration;
        m_ImageSize = originalMetadata.m_ImageSize;

        return false;
    }

    bool VideoArtwork::initFromStorageUnsafe(const MetadataIO::CachedArtwork &cachedArtwork) {
        // this time we need to overwrite from storage
        // if (m_CodecName.isEmpty()) {
        if (!cachedArtwork.m_CodecName.isEmpty()) {
            m_CodecName = cachedArtwork.m_CodecName;
        }

        initializeThumbnailPath(cachedArtwork.m_ThumbnailPath);

        return false;
    }
}

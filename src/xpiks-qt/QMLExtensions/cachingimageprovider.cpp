/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "cachingimageprovider.h"
#include "../Common/logging.h"
#include "../QMLExtensions/imagecachingservice.h"
#include "../Helpers/stringhelper.h"

#define RECACHE true

namespace QMLExtensions {
    QString prepareUrl(const QString &url) {
        QString id;

        if (url.contains(QChar('%'))) {
            id = Helpers::stringPercentDecode(url);
        } else {
            id = url;
        }

        return id;
    }

    QImage CachingImageProvider::requestImage(const QString &url, QSize *size, const QSize &requestedSize) {
        Q_ASSERT(!url.isEmpty());
        if (url.isEmpty()) { return QImage(); }

        const QString id = prepareUrl(url);
        LOG_DEBUG << "Requesting file:" << id;

        QString cachedPath;
        bool needsUpdate = false;

        if (m_ImageCachingService->tryGetCachedImage(id, requestedSize, cachedPath, needsUpdate)) {
            if (needsUpdate) {
                m_ImageCachingService->cacheImage(id, requestedSize, RECACHE);
            }

            QImage cachedImage;
            bool loaded = cachedImage.load(cachedPath);
            if (loaded && !cachedImage.isNull()) {
                *size = cachedImage.size();
                return cachedImage;
            }
        }

        LOG_DEBUG << "Not found properly cached:" << id;

        QImage originalImage(id);
        QImage result;

        if (requestedSize.isValid()) {
            m_ImageCachingService->cacheImage(id, requestedSize);
            result = originalImage.scaled(requestedSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        } else {
            LOG_WARNING << "Size is invalid:" << requestedSize.width() << "x" << requestedSize.height();
            m_ImageCachingService->cacheImage(id);
            result = originalImage.scaled(m_ImageCachingService->getDefaultSize(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }

        *size = result.size();
        return result;
    }
}

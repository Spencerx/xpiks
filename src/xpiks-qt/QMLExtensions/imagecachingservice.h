/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef IMAGECACHINGSERVICE_H
#define IMAGECACHINGSERVICE_H

#include <QObject>
#include <QString>
#include <QVector>
#include <QSize>
#include <vector>
#include <memory>
#include "../Common/baseentity.h"
#include "../Common/iservicebase.h"
#include "../Common/isystemenvironment.h"

namespace Models {
    class ArtworkMetadata;
    class ArtworkMetadataLocker;
}

namespace MetadataIO {
    class ArtworksSnapshot;
}

class QScreen;

namespace QMLExtensions {
    class ImageCachingWorker;

    class ImageCachingService : public QObject, public Common::BaseEntity
    {
        Q_OBJECT
    public:
        explicit ImageCachingService(Common::ISystemEnvironment &environment, QObject *parent = 0);

    public:
        void startService(const std::shared_ptr<Common::ServiceStartParams> &params);
        void stopService();
        void upgradeCacheStorage();

    public:
        const QSize &getDefaultSize() const { return m_DefaultSize; }

    public:
        void setScale(qreal scale);
        void cacheImage(const QString &key, const QSize &requestedSize, bool recache=false);
        void cacheImage(const QString &key);
        void generatePreviews(const MetadataIO::ArtworksSnapshot &snapshot);
        bool tryGetCachedImage(const QString &key, const QSize &requestedSize, QString &cached, bool &needsUpdate);

    private:
        void updateDefaultSize();

    public slots:
        void screenChangedHandler(QScreen *screen);
        void dpiChanged(qreal someDPI);

    private:
        Common::ISystemEnvironment &m_Environment;
        ImageCachingWorker *m_CachingWorker;
        QSize m_DefaultSize;
        volatile bool m_IsCancelled;
        qreal m_Scale;
    };
}

#endif // IMAGECACHINGSERVICE_H

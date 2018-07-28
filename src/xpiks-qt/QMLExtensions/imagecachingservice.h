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
#include <QSize>
#include <memory>
#include <Common/isystemenvironment.h>
#include <Common/irefcountedobject.h>

namespace Artworks {
    class ArtworkMetadata;
    using ArtworkMetadataLocker = Common::HoldLocker<ArtworkMetadata>;
    class ArtworksSnapshot;
}

namespace Storage {
    class IDatabaseManager;
}

namespace Services {
    class ServiceStartParams;
}

namespace Helpers {
    class AsyncCoordinator;
}

class QScreen;

namespace QMLExtensions {
    class ImageCachingWorker;

    class ImageCachingService : public QObject
    {
        Q_OBJECT
    public:
        explicit ImageCachingService(Common::ISystemEnvironment &environment,
                                     Storage::IDatabaseManager &dbManager,
                                     QObject *parent = 0);

    public:
        void startService(Helpers::AsyncCoordinator &coordinator);
        void stopService();
        void upgradeCacheStorage();

    public:
        const QSize &getDefaultSize() const { return m_DefaultSize; }

    public:
        void setScale(qreal scale);
        void cacheImage(const QString &key, const QSize &requestedSize, bool recache=false);
        void cacheImage(const QString &key);
        void generatePreviews(const Artworks::ArtworksSnapshot &snapshot);
        bool tryGetCachedImage(const QString &key, const QSize &requestedSize, QString &cached, bool &needsUpdate);

    private:
        void updateDefaultSize();

    public slots:
        void screenChangedHandler(QScreen *screen);
        void dpiChanged(qreal someDPI);

    private:
        Common::ISystemEnvironment &m_Environment;
        Storage::IDatabaseManager &m_DatabaseManager;
        ImageCachingWorker *m_CachingWorker;
        QSize m_DefaultSize;
        volatile bool m_IsCancelled;
        qreal m_Scale;
    };
}

#endif // IMAGECACHINGSERVICE_H

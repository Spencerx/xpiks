/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef VIDEOCACHINGWORKER_H
#define VIDEOCACHINGWORKER_H

#include <cstdint>
#include <memory>
#include <vector>

#include <QObject>
#include <QString>
#include <QVector>
#include <QtGlobal>

#include "Common/itemprocessingworker.h"
#include "QMLExtensions/dbvideocacheindex.h"

class QImage;

namespace Common {
    class ISystemEnvironment;
}

namespace Services {
    class ArtworksUpdateHub;
}

namespace MetadataIO {
    class MetadataIOService;
}

namespace Storage {
    class IDatabaseManager;
}

namespace QMLExtensions {
    class ImageCachingService;
    class VideoCacheRequest;

    class VideoCachingWorker : public QObject, public Common::ItemProcessingWorker<VideoCacheRequest>
    {
        Q_OBJECT
    public:
        explicit VideoCachingWorker(Common::ISystemEnvironment &environment,
                                    Storage::IDatabaseManager &dbManager,
                                    ImageCachingService &imageCachingService,
                                    Services::ArtworksUpdateHub &updateHub,
                                    MetadataIO::MetadataIOService &metadataIOService,
                                    QObject *parent = 0);

    protected:
        virtual bool initWorker() override;
        virtual std::shared_ptr<void> processWorkItem(WorkItem &workItem) override;
        virtual void processOneItem(std::shared_ptr<VideoCacheRequest> &item) override;

    private:
        bool createThumbnail(std::shared_ptr<VideoCacheRequest> &item, std::vector<uint8_t> &buffer, int &width, int &height);

    protected:
        virtual void onQueueIsEmpty() override { emit queueIsEmpty(); }
        virtual void onWorkerStopped() override;

    public slots:
        void process() { doWork(); }
        void cancel() { stopWorking(); }

    signals:
        void stopped();
        void queueIsEmpty();

    public:
        bool tryGetVideoThumbnail(const QString &key, QString &cachedPath, bool &needsUpdate);

    private:
        bool saveThumbnail(QImage &image, const QString &originalPath, bool isQuickThumbnail, QString &thumbnailPath);
        void cacheImage(const QString &thumbnailPath);
        void applyThumbnail(std::shared_ptr<VideoCacheRequest> &item, const QString &thumbnailPath, bool recacheArtwork);
        void saveIndex();
        bool checkLockedIO(std::shared_ptr<VideoCacheRequest> &item);
        bool checkProcessed(std::shared_ptr<VideoCacheRequest> &item);

    private:
        Common::ISystemEnvironment &m_Environment;
        ImageCachingService &m_ImageCachingService;
        Services::ArtworksUpdateHub &m_ArtworksUpdateHub;
        MetadataIO::MetadataIOService &m_MetadataIOService;
        volatile int m_ProcessedItemsCount;
        qreal m_Scale;
        QString m_VideosCacheDir;
        DbVideoCacheIndex m_Cache;
        QVector<int> m_RolesToUpdate;
    };
}

#endif // VIDEOCACHINGWORKER_H

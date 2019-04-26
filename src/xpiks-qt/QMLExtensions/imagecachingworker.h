/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef IMAGECACHINGWORKER_H
#define IMAGECACHINGWORKER_H

#include <memory>

#include <QObject>
#include <QString>
#include <QtGlobal>

#include "Common/itemprocessingworker.h"
#include "QMLExtensions/dbimagecacheindex.h"

class QSize;

namespace Common {
    class ISystemEnvironment;
}

namespace Helpers {
    class AsyncCoordinator;
}

namespace Storage {
    class IDatabaseManager;
}

namespace QMLExtensions {
    class ImageCacheRequest;

    class ImageCachingWorker : public QObject, public Common::ItemProcessingWorker<ImageCacheRequest>
    {
        Q_OBJECT
    public:
        ImageCachingWorker(Common::ISystemEnvironment &environment,
                           Helpers::AsyncCoordinator &initCoordinator,
                           Storage::IDatabaseManager &dbManager,
                           QObject *parent=0);

    protected:
        virtual bool initWorker() override;
        virtual std::shared_ptr<void> processWorkItem(WorkItem &workItem) override;
        virtual void processOneItem(const std::shared_ptr<ImageCacheRequest> &item) override;

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
        void setScale(qreal scale) { m_Scale = scale; }
        bool tryGetCachedImage(const QString &key, const QSize &requestedSize,
                               QString &cached, bool &needsUpdate);
        bool upgradeCacheStorage();

    private:
        void saveIndex();
        bool isProcessed(const std::shared_ptr<ImageCacheRequest> &item);

    private:
        Common::ISystemEnvironment &m_Environment;
        Helpers::AsyncCoordinator &m_InitCoordinator;
        volatile int m_ProcessedItemsCount;
        DbImageCacheIndex m_Cache;
        qreal m_Scale;
        QString m_ImagesCacheDir;
    };
}

#endif // IMAGECACHINGWORKER_H

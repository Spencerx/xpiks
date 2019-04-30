/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef WARNINGSCHECKINGWORKER_H
#define WARNINGSCHECKINGWORKER_H

#include <memory>

#include <QObject>
#include <QString>

#include "Common/itemprocessingworker.h"

namespace Warnings {
    class WarningsSettingsModel;
    class IWarningsItem;

    class WarningsCheckingWorker:
        public QObject, public Common::ItemProcessingWorker<IWarningsItem>
    {
    Q_OBJECT

    public:
        WarningsCheckingWorker(WarningsSettingsModel &warningsSettingsModel, QObject *parent=nullptr);

    protected:
        virtual bool initWorker() override;
        virtual std::shared_ptr<void> processWorkItem(WorkItem &workItem) override;

    protected:
        virtual void onQueueIsEmpty() override { /* Notify only on batches */ /* emit queueIsEmpty(); */ }
        virtual void onWorkerStopped() override { emit stopped(); }

    public slots:
        void process() { doWork(); }
        void cancel() { stopWorking(); }

    signals:
        void stopped();
        void queueIsEmpty();

    private:
        WarningsSettingsModel &m_WarningsSettingsModel;
    };
}

#endif // WARNINGSCHECKINGWORKER_H

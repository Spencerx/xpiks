/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2017 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef WARNINGSCHECKINGWORKER_H
#define WARNINGSCHECKINGWORKER_H

#include <QObject>
#include "../Common/itemprocessingworker.h"
#include "warningsitem.h"

namespace Warnings {
    class WarningsSettingsModel;

    class WarningsCheckingWorker:
        public QObject, public Common::ItemProcessingWorker<WarningsItem>
    {
    Q_OBJECT

    public:
        WarningsCheckingWorker(WarningsSettingsModel *warningsSettingsModel, QObject *parent=0);

    protected:
        virtual bool initWorker() override;
        virtual void processOneItem(std::shared_ptr<WarningsItem> &item) override;

    private:
        void initValuesFromSettings();

    protected:
        virtual void notifyQueueIsEmpty() override { emit queueIsEmpty(); }
        virtual void workerStopped() override { emit stopped(); }

    public slots:
        void process() { doWork(); }
        void cancel() { stopWorking(); }

    signals:
        void stopped();
        void queueIsEmpty();

    private:
        Common::WarningFlags checkDimensions(std::shared_ptr<WarningsItem> &wi) const;
        Common::WarningFlags checkKeywords(std::shared_ptr<WarningsItem> &wi) const;
        Common::WarningFlags checkDescription(std::shared_ptr<WarningsItem> &wi) const;
        Common::WarningFlags checkTitle(std::shared_ptr<WarningsItem> &wi) const;
        Common::WarningFlags checkSpelling(std::shared_ptr<WarningsItem> &wi) const;
        Common::WarningFlags checkDuplicates(std::shared_ptr<WarningsItem> &wi) const;

    private:
        WarningsSettingsModel *m_WarningsSettingsModel;
    };
}

#endif // WARNINGSCHECKINGWORKER_H

/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "warningscheckingworker.h"

#include <QThread>
#include <QtDebug>

#include "Common/itemprocessingworker.h"
#include "Common/logging.h"
#include "Services/Warnings/iwarningsitem.h"
#include "warningssettingsmodel.h"

#define WARNINGS_WORKER_SLEEP_INTERVAL 500
#define WARNINGS_DELAY_PERIOD 50

namespace Warnings {
    WarningsCheckingWorker::WarningsCheckingWorker(WarningsSettingsModel &warningsSettingsModel,
                                                   QObject *parent):
        QObject(parent),
        ItemProcessingWorker(WARNINGS_DELAY_PERIOD),
        m_WarningsSettingsModel(warningsSettingsModel)
    {
    }

    bool WarningsCheckingWorker::initWorker() {
        LOG_DEBUG << "#";
        return true;
    }

    std::shared_ptr<void> WarningsCheckingWorker::processWorkItem(WorkItem &workItem) {
        if (workItem.isSeparator()) {
            emit queueIsEmpty();
        } else {
            workItem.m_Item->checkWarnings(m_WarningsSettingsModel);

            if (workItem.isMilestone()) {
                QThread::msleep(WARNINGS_WORKER_SLEEP_INTERVAL);
            }
        }

        return std::shared_ptr<void>();
    }
}

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
#include "../Common/logging.h"
#include "../Common/flags.h"
#include "warningssettingsmodel.h"
#include "warningsitem.h"

#define WARNINGS_WORKER_SLEEP_INTERVAL 500
#define WARNINGS_DELAY_PERIOD 50

namespace Warnings {
    WarningsCheckingWorker::WarningsCheckingWorker(WarningsSettingsModel *warningsSettingsModel,
                                                   QObject *parent):
        QObject(parent),
        ItemProcessingWorker(WARNINGS_DELAY_PERIOD),
        m_WarningsSettingsModel(warningsSettingsModel)
    {
        Q_ASSERT(warningsSettingsModel != nullptr);
    }

    bool WarningsCheckingWorker::initWorker() {
        LOG_DEBUG << "#";
        return true;
    }

    void WarningsCheckingWorker::processOneItemEx(std::shared_ptr<IWarningsItem> &item, batch_id_t batchID, Common::flag_t flags) {
        if (getIsSeparatorFlag(flags)) {
            emit queueIsEmpty();
        } else {
            ItemProcessingWorker::processOneItemEx(item, batchID, flags);

            if (getIsMilestone(flags)) {
                QThread::msleep(WARNINGS_WORKER_SLEEP_INTERVAL);
            }
        }
    }

    void WarningsCheckingWorker::processOneItem(std::shared_ptr<IWarningsItem> &item) {
        item->checkWarnings(m_WarningsSettingsModel);
    }
}

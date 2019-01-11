/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "loggingworker.h"

#include <QThread>

#include "Helpers/logger.h"

namespace Helpers {
    LoggingWorker::LoggingWorker(QObject *parent) :
        QObject(parent),
        m_Cancel(false)
    {
    }

    void LoggingWorker::process() {
        Logger &logger = Logger::getInstance();
        const int sleepTimeout = 700;

        while (!m_Cancel) {
            logger.flush();
            QThread::msleep(sleepTimeout);
        }

        emit stopped();
    }

    void LoggingWorker::cancel() {
        m_Cancel = true;

        Logger &logger = Logger::getInstance();
        logger.stop();
    }
}

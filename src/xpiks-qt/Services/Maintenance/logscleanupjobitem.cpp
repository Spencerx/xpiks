/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "logscleanupjobitem.h"

#include <QSet>
#include <QStringList>

#include "Common/isystemenvironment.h"
#include "Common/logging.h"
#include "Helpers/logger.h"
#include "Services/Maintenance/filescleanup.h"

#define MAX_LOGS_SIZE_MB 10
#define MAX_LOGS_SIZE_BYTES (MAX_LOGS_SIZE_MB * 1024 * 1024)
#define MAX_LOGS_AGE_DAYS 60
#define MAX_LOGS_NUMBER 100

namespace Maintenance {
    LogsCleanupJobItem::LogsCleanupJobItem(Common::ISystemEnvironment &environment):
        m_LogFileDir(environment.path({"logs"}))
    { }

    void LogsCleanupJobItem::processJob() {
        LOG_DEBUG << "#";
        Helpers::Logger &logger = Helpers::Logger::getInstance();
        const QString logFilePath = logger.getLogFilePath();

        FilesCleanup(m_LogFileDir,
                     QStringList() << "xpiks-qt-*.log",
                     QSet<QString>() << logFilePath)
            .Run(MAX_LOGS_NUMBER, MAX_LOGS_AGE_DAYS, MAX_LOGS_SIZE_BYTES);
    }
}


/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "logger.h"
#include <QStringList>
#include <QString>
#include <QMutexLocker>
#include <QMutex>
#include <QFile>
#include <QTextStream>
#include <QTimer>
#include <QDir>
#include <QDateTime>
#include <QStandardPaths>
#include <QElapsedTimer>
#include <iostream>
#include <string>
#include <ctime>
#include "../Common/defines.h"

#define MIN_FIRE_SIZE 20
#define LOGGING_TIMEOUT 5

namespace Helpers {
    void Logger::log(const QString &message) {
        if (!m_Stopped) {
            doLog(message);
        }
    }

    void Logger::flush() {
        if (m_Stopped) { return; }

        QMutexLocker flushLocker(&m_FlushMutex);

        while (m_QueueFlushFrom->isEmpty()) {
            QMutexLocker logLocker(&m_LogMutex);

            if (m_QueueLogTo->isEmpty()) {
                m_AnyLogsToFlush.wait(&m_LogMutex);
            } else {
                qSwap(m_QueueLogTo, m_QueueFlushFrom);
            }
        }

        Q_ASSERT(m_QueueFlushFrom->length() > 0);
        flushStream(m_QueueFlushFrom);
    }

    void Logger::stop() {
        m_Stopped = true;

        // will make waiting flush() call unblocked if any
        doLog("Logging stopped.");

        QMutexLocker flushLocker(&m_FlushMutex);
        flushStream(m_QueueFlushFrom);

        {
            QMutexLocker logLocker(&m_LogMutex);

            if (!m_QueueLogTo->isEmpty()) {
                qSwap(m_QueueLogTo, m_QueueFlushFrom);
            }
        }

        flushStream(m_QueueFlushFrom);
    }

#ifdef INTEGRATION_TESTS
    void Logger::emergencyFlush() {
        flushStream(&m_LogsStorage[0]);
        flushStream(&m_LogsStorage[1]);
    }
#endif

    void Logger::doLog(const QString &message) {
        QMutexLocker locker(&m_LogMutex);
        m_QueueLogTo->append(message);
        m_AnyLogsToFlush.wakeOne();
    }

    void Logger::flushStream(QStringList *logItems) {
        Q_ASSERT(logItems != nullptr);
        if (logItems->empty()) { return; }

#ifdef WITH_LOGS
        QFile outFile(m_LogFilepath);
        if (outFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append)) {
            QTextStream ts(&outFile);
            ts.setCodec("UTF-8");

            int size = logItems->size();
            for (int i = 0; i < size; ++i) {
                const QString &line = logItems->at(i);
                ts << line;
                endl(ts);
            }
        }
#endif

#ifdef WITH_STDOUT_LOGS
        int size = logItems->size();
        for (int i = 0; i < size; ++i) {
            const QString &line = logItems->at(i);
            std::cout << line.toLocal8Bit().data() << std::endl;
        }

        std::cout.flush();
#endif

        logItems->clear();
    }
}

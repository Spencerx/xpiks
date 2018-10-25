/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "logsmodel.h"

#include <QFile>
#include <QIODevice>
#include <QString>
#include <QThread>
#include <QtDebug>
#include <QtGlobal>

#include "Common/logging.h"
#include "Helpers/logger.h"
#include "Helpers/loggingworker.h"
#include "Helpers/stringhelper.h"

namespace Models {
    LogsModel::LogsModel(QObject *parent) :
        QObject(parent),
        m_LoggingWorker(new Helpers::LoggingWorker())
    {
#ifdef WITH_LOGS
        m_WithLogs = true;
#else
        m_WithLogs = false;
#endif
    }

    void LogsModel::startLogging() {
        QThread *loggingThread = new QThread();
        m_LoggingWorker->moveToThread(loggingThread);

        QObject::connect(loggingThread, &QThread::started, m_LoggingWorker, &Helpers::LoggingWorker::process);
        QObject::connect(m_LoggingWorker, &Helpers::LoggingWorker::stopped, loggingThread, &QThread::quit);

        QObject::connect(m_LoggingWorker, &Helpers::LoggingWorker::stopped, m_LoggingWorker, &Helpers::LoggingWorker::deleteLater);
        QObject::connect(loggingThread, &QThread::finished, loggingThread, &QThread::deleteLater);

        loggingThread->start(QThread::LowPriority);
    }

    void LogsModel::stopLogging() {
        m_LoggingWorker->cancel();
    }

    QString LogsModel::getAllLogsText(bool moreLogs) {
        QString result;
#ifdef WITH_LOGS
        Helpers::Logger &logger = Helpers::Logger::getInstance();
        QString logFilePath = logger.getLogFilePath();
        QFile f(logFilePath);

        if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
            // 1000 - do not load the UI
            // advanced users will open logs it notepad
            int numberOfLines = moreLogs ? 1000 : 100;
            QString text = QString::fromUtf8(f.readAll());
            f.close();
            result = Helpers::getLastNLines(text, numberOfLines);
        }
#else
        Q_UNUSED(moreLogs);
        result = QString::fromLatin1("Logs are not available in this version");
#endif

#ifndef QT_DEBUG
        result = Encryption::rot13plus(result);
#endif

        return result;
    }

    void LogsModel::clearLogsExtract() {
        LOG_DEBUG << "#";
        m_LogsExtract.clear();
        emit logsExtractChanged();
    }

    void LogsModel::updateLogs(bool moreLogs) {
        m_LogsExtract = getAllLogsText(moreLogs);
        emit logsExtractChanged();
    }
}

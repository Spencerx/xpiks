/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2017 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "logsmodel.h"
#include "../Helpers/loggingworker.h"
#include <QThread>
#include <QString>
#include <QFile>
#include <QDir>
#include <QDateTime>
#include <QTextStream>
#include <QStandardPaths>
#include "../Helpers/stringhelper.h"
#include "../Helpers/logger.h"
#include "../Helpers/loghighlighter.h"
#include "../Common/defines.h"

namespace Models {

    LogsModel::LogsModel(QMLExtensions::ColorsModel *colorsModel, QObject *parent) :
        QObject(parent),
        m_LoggingWorker(new Helpers::LoggingWorker()),
        m_ColorsModel(colorsModel)
    {
#ifdef WITH_LOGS
        m_WithLogs = true;
#else
        m_WithLogs = false;
#endif
    }

    LogsModel::~LogsModel() {
    }

    void LogsModel::startLogging() {
        QThread *loggingThread = new QThread();
        m_LoggingWorker->moveToThread(loggingThread);

        QObject::connect(loggingThread, SIGNAL(started()), m_LoggingWorker, SLOT(process()));
        QObject::connect(m_LoggingWorker, SIGNAL(stopped()), loggingThread, SLOT(quit()));

        QObject::connect(m_LoggingWorker, SIGNAL(stopped()), m_LoggingWorker, SLOT(deleteLater()));
        QObject::connect(loggingThread, SIGNAL(finished()), loggingThread, SLOT(deleteLater()));

        loggingThread->start();
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
            result = Helpers::getLastNLines(text, numberOfLines);
            f.close();
        }
#else
        Q_UNUSED(moreLogs);
        result = QString::fromLatin1("Logs are not available in this version");
#endif
        return result;

    }

    void LogsModel::initLogHighlighting(QQuickTextDocument *document) {
        Helpers::LogHighlighter *highlighter = new Helpers::LogHighlighter(m_ColorsModel, document->textDocument());
        Q_UNUSED(highlighter);
    }
}

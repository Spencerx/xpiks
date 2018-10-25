/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef LOGSMODEL
#define LOGSMODEL

#include <QObject>
#include <QString>

namespace Helpers {
    class LoggingWorker;
}

namespace Models {
    class LogsModel : public QObject {
        Q_OBJECT
        Q_PROPERTY(bool withLogs READ getWithLogs CONSTANT)
        Q_PROPERTY(QString logsExtract READ getLogsExtract NOTIFY logsExtractChanged)

    public:
        LogsModel(QObject *parent=nullptr);

    public:
        const QString &getLogsExtract() const { return m_LogsExtract; }
        bool getWithLogs() const { return m_WithLogs; }

    public:
        void startLogging();
        void stopLogging();

    public:
        QString getAllLogsText(bool moreLogs=false);
        void updateLogs(bool moreLogs=false);

    public:
        Q_INVOKABLE void clearLogsExtract();

    signals:
        void logsExtractChanged();

    private:
        Helpers::LoggingWorker *m_LoggingWorker;
        QString m_LogsExtract;
        bool m_WithLogs;
    };
}

#endif // LOGSMODEL


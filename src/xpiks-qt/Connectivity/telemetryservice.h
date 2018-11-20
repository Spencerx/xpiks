/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef TELEMETRYSERVICE_H
#define TELEMETRYSERVICE_H

#include <atomic>

#include <QObject>
#include <QString>

#include "Common/messages.h"
#include "Common/types.h"
#include "Connectivity/analyticsuserevent.h"

namespace Models {
    class SettingsModel;
    class SwitcherModel;
}

namespace Connectivity {
    class TelemetryWorker;

    class TelemetryService:
            public QObject,
            public Common::MessagesTarget<Common::NamedType<UserAction>>
    {
        Q_OBJECT
    public:
        TelemetryService(Models::SwitcherModel &switcher,
                         Models::SettingsModel &settingsModel,
                         QObject *parent=nullptr);

    public:
        void initialize();
        void startReporting();
        void stopReporting(bool immediately=true);

    public:
        virtual void handleMessage(Common::NamedType<UserAction> const &event) override;

    private:
        bool isRunning();
        void ensureUserIdExists();
        void doStartReporting();
        bool getIsTelemetryEnabled();
        void doReportAction(UserAction action);

    public:
        void reportAction(UserAction action);
        void setEndpoint(const QString &endpoint);
        void setInterfaceLanguage(const QString &language) { m_InterfaceLanguage = language; }

    public slots:
        void changeReporting(bool value);

    private slots:
        void workerDestroyed(QObject* object);

    signals:
        void cancelAllQueries();

    private:
        TelemetryWorker *m_TelemetryWorker;
        Models::SwitcherModel &m_Switcher;
        Models::SettingsModel &m_SettingsModel;
        QString m_ReportingEndpoint;
        QString m_UserAgentId;
        QString m_InterfaceLanguage;
        std::atomic_bool m_IsStopped;
    };
}

#endif // TELEMETRYSERVICE_H


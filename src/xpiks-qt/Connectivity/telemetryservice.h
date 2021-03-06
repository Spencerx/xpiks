/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
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
#include "Connectivity/telemetryconfig.h"

namespace Models {
    class SettingsModel;
    class SwitcherModel;
}

namespace Common {
    class ISystemEnvironment;
}

namespace Connectivity {
    class TelemetryWorker;
    class IRequestsService;

    class TelemetryService:
            public QObject,
            public Common::MessagesTarget<Common::NamedType<EventType>>
    {
        Q_OBJECT
    public:
        TelemetryService(Common::ISystemEnvironment &environment,
                         Models::SwitcherModel &switcher,
                         Models::SettingsModel &settingsModel,
                         QObject *parent=nullptr);

    public:
        void initialize(IRequestsService &requestsService);
        void startReporting();
        void stopReporting(bool immediately=true);

    public:
        virtual void handleMessage(Common::NamedType<EventType> const &event) override;

    private:
        bool isRunning();
        void ensureUserIdExists();
        void doStartReporting();
        bool getIsTelemetryEnabled();
        void doReportAction(EventType action);

    public:
        void reportAction(EventType action);
        void setInterfaceLanguage(const QString &language) { m_InterfaceLanguage = language; }

    public slots:
        void changeReporting(bool value);

    private slots:
        void workerDestroyed(QObject* object);

    signals:
        void cancelAllQueries();

    private:
        TelemetryWorker *m_TelemetryWorker;
        TelemetryConfig m_Config;
        Models::SwitcherModel &m_Switcher;
        Models::SettingsModel &m_SettingsModel;
        QString m_UserAgentId;
        QString m_InterfaceLanguage;
        std::atomic_bool m_IsStopped;
    };
}

#endif // TELEMETRYSERVICE_H


/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef TELEMETRYWORKER_H
#define TELEMETRYWORKER_H

#include <memory>

#include <QObject>
#include <QString>

#include "Common/itemprocessingworker.h"

namespace Models {
    class SettingsModel;
}

namespace Connectivity {
    class AnalyticsUserEvent;
    class TelemetryConfig;

    class TelemetryWorker : public QObject, public Common::ItemProcessingWorker<AnalyticsUserEvent>
    {
        Q_OBJECT
    public:
        TelemetryWorker(const QString &userAgent,
                        const QString &interfaceLanguage,
                        TelemetryConfig &config,
                        Models::SettingsModel &settingsModel);

    protected:
        virtual bool initWorker() override;
        virtual std::shared_ptr<void> processWorkItem(WorkItem &workItem) override;
        virtual void processOneItem(const std::shared_ptr<AnalyticsUserEvent> &item) override;

    private:
        bool sendOneReport(const QString &resource, const QString &payload);

    protected:
        virtual void onQueueIsEmpty() override { emit queueIsEmpty(); }
        virtual void onWorkerStopped() override { emit stopped(); }

    public slots:
        void process() { doWork(); }
        void cancel() { stopWorking(); }

    private slots:
        void onConfigUpdated();

    signals:
        void stopped();
        void queueIsEmpty();
        void cancelAllQueries();

    private:
        QString m_UserAgentId;
        QString m_ReportingEndpoint;
        QString m_InterfaceLanguage;
        TelemetryConfig &m_Config;
        Models::SettingsModel &m_SettingsModel;
    };
}

#endif // TELEMETRYWORKER_H

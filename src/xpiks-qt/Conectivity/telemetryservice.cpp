/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2017 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "telemetryservice.h"
#include <QThread>
#include "telemetryworker.h"
#include "../Common/defines.h"
#include "../Common/version.h"

namespace Conectivity {
    TelemetryService::TelemetryService(const QString &userId, bool telemetryEnabled, QObject *parent) :
        QObject(parent),
        m_TelemetryWorker(nullptr),
        m_UserAgentId(userId),
        m_InterfaceLanguage("en_US"),
        m_TelemetryEnabled(telemetryEnabled),
        m_RestartRequired(false)
    {
        LOG_INFO << "Enabled:" << telemetryEnabled;
    }

    void TelemetryService::startReporting() {
        if (m_TelemetryEnabled) {
            doStartReporting();
        } else {
            LOG_WARNING << "Telemetry is disabled";
        }
    }

    void TelemetryService::stopReporting(bool immediately) {
        LOG_DEBUG << "#";

        if (m_TelemetryWorker != nullptr) {
            m_TelemetryWorker->stopWorking(immediately);
        } else {
            LOG_WARNING << "TelemetryWorker is NULL";
        }
    }

    void TelemetryService::doStartReporting() {
        Q_ASSERT(m_TelemetryWorker == nullptr);
        LOG_DEBUG << "#";

        m_TelemetryWorker = new TelemetryWorker(m_UserAgentId, m_ReportingEndpoint, m_InterfaceLanguage);

        QThread *thread = new QThread();
        m_TelemetryWorker->moveToThread(thread);

        QObject::connect(thread, SIGNAL(started()), m_TelemetryWorker, SLOT(process()));
        QObject::connect(m_TelemetryWorker, SIGNAL(stopped()), thread, SLOT(quit()));

        QObject::connect(m_TelemetryWorker, SIGNAL(stopped()), m_TelemetryWorker, SLOT(deleteLater()));
        QObject::connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));

        QObject::connect(this, SIGNAL(cancelAllQueries()), m_TelemetryWorker, SIGNAL(cancelAllQueries()));

        thread->start();
    }

    void TelemetryService::reportAction(UserAction action) {
        if (m_TelemetryEnabled) {
            if (m_TelemetryWorker != nullptr) {
                std::shared_ptr<AnalyticsUserEvent> item(new AnalyticsUserEvent(action));
                m_TelemetryWorker->submitItem(item);
            } else {
                LOG_WARNING << "Worker is null";
            }
        } else {
            LOG_DEBUG << "Telemetry disabled";
        }
    }

    void TelemetryService::changeReporting(bool value) {
        LOG_INFO << value;

        if (m_TelemetryEnabled != value) {
            m_TelemetryEnabled = value;

            if (m_TelemetryEnabled) {
                LOG_DEBUG << "Telemetry enabled";
                startReporting();
            } else {
                LOG_DEBUG << "Telemetry disabled";
                reportAction(UserAction::TurnOffTelemetry);
                stopReporting(false);
            }
        }
    }

    void TelemetryService::workerDestroyed(QObject *object) {
        Q_UNUSED(object);
        LOG_DEBUG << "#";
        m_TelemetryWorker = nullptr;

        if (m_RestartRequired) {
            LOG_INFO << "Restarting worker...";
            startReporting();
            m_RestartRequired = false;
        }
    }

    void TelemetryService::setEndpoint(const QString &endpoint) {
        m_ReportingEndpoint = endpoint;
    }
}

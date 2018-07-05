/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "telemetryservice.h"
#include <QThread>
#include <QUuid>
#include "telemetryworker.h"
#include <Common/defines.h>
#include <Common/version.h>
#include <Commands/appmessages.h>

namespace Connectivity {
    TelemetryService::TelemetryService(Models::SwitcherModel &switcher,
                                       Models::SettingsModel &settingsModel,
                                       Commands::AppMessages &messages,
                                       QObject *parent) :
        QObject(parent),
        m_TelemetryWorker(nullptr),
        m_Switcher(switcher),
        m_SettingsModel(settingsModel),
        m_InterfaceLanguage("en_US")
    {
        messages
                .ofType<int>()
                .withID(Commands::AppMessages::Telemetry)
                .addListener(std::bind(&TelemetryService::reportAction, this,
                                       std::placeholders::_1));
    }

    void TelemetryService::initialize() {
#ifndef INTEGRATION_TESTS
    ensureUserIdExists();

    QString userId = m_SettingsModel.getUserAgentId();
    userId.remove(QRegExp("[{}-]."));

    m_UserAgentId = userId;
#else
    m_UserAgentId = "1234567890";
    Q_ASSERT(!getIsTelemetryEnabled());
#endif
    }

    void TelemetryService::startReporting() {
        if (getIsTelemetryEnabled()) {
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

    void TelemetryService::ensureUserIdExists() {
        QString userID = m_SettingsModel.getUserAgentId();
        QUuid latest(userID);
        if (userID.isEmpty()
                || (latest.isNull())
                || (latest.version() == QUuid::VerUnknown)) {
            QUuid uuid = QUuid::createUuid();
            m_SettingsModel.setUserAgentId(uuid.toString());
        }
    }

    void TelemetryService::doStartReporting() {
        Q_ASSERT(m_TelemetryWorker == nullptr);
        LOG_DEBUG << "#";

        m_TelemetryWorker = new TelemetryWorker(m_UserAgentId, m_ReportingEndpoint, m_InterfaceLanguage);

        QThread *thread = new QThread();
        m_TelemetryWorker->moveToThread(thread);

        QObject::connect(thread, &QThread::started, m_TelemetryWorker, &TelemetryWorker::process);
        QObject::connect(m_TelemetryWorker, &TelemetryWorker::stopped, thread, &QThread::quit);

        QObject::connect(m_TelemetryWorker, &TelemetryWorker::stopped, m_TelemetryWorker, &TelemetryWorker::deleteLater);
        QObject::connect(thread, &QThread::finished, thread, &QThread::deleteLater);

        QObject::connect(this, &TelemetryService::cancelAllQueries, m_TelemetryWorker, &TelemetryWorker::cancelAllQueries);

        thread->start(QThread::LowestPriority);
    }

    bool TelemetryService::getIsTelemetryEnabled() {
#ifndef INTEGRATION_TESTS
        return m_SettingsModel.getIsTelemetryEnabled() && m_Switcher.getIsTelemetryEnabled();
#else
        return false;
#endif
    }

    void TelemetryService::doReportAction(UserAction action) {
        if (m_TelemetryWorker != nullptr) {
            std::shared_ptr<AnalyticsUserEvent> item(new AnalyticsUserEvent(action));
            m_TelemetryWorker->submitItem(item);
        } else {
            LOG_WARNING << "Worker is null";
        }
    }

    void TelemetryService::reportAction(int action) {
        if (getIsTelemetryEnabled()) {
            doReportAction((UserAction)action);
        } else {
            LOG_DEBUG << "Telemetry disabled";
        }
    }

    void TelemetryService::changeReporting(bool value) {
        LOG_INFO << value;

        if (value == false) {
            doReportAction(UserAction::TurnOffTelemetry);
        }
    }

    void TelemetryService::workerDestroyed(QObject *object) {
        Q_UNUSED(object);
        LOG_DEBUG << "#";
        m_TelemetryWorker = nullptr;
    }

    void TelemetryService::setEndpoint(const QString &endpoint) {
        m_ReportingEndpoint = endpoint;
    }
}

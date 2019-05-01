/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "telemetryservice.h"

#include <memory>

#include <QRegExp>
#include <QThread>
#include <QUuid>
#include <QtDebug>
#include <QtGlobal>

#include "Common/logging.h"
#include "Common/types.h"
#include "Connectivity/analyticsuserevent.h"
#include "Connectivity/telemetryworker.h"
#include "Models/settingsmodel.h"
#include "Models/switchermodel.h"

namespace Connectivity {
    TelemetryService::TelemetryService(Models::SwitcherModel &switcher,
                                       Models::SettingsModel &settingsModel,
                                       QObject *parent) :
        QObject(parent),
        m_TelemetryWorker(nullptr),
        m_Switcher(switcher),
        m_SettingsModel(settingsModel),
        m_InterfaceLanguage("en_US"),
        m_IsStopped(false)
    {
    }

    void TelemetryService::initialize() {
        ensureUserIdExists();

        QString userId = m_SettingsModel.getUserAgentId();
        userId.remove(QRegExp("[{}-]."));

        m_UserAgentId = userId;
    }

    void TelemetryService::startReporting() {
        LOG_VERBOSE_OR_DEBUG << "settings telemetry:" << m_SettingsModel.getCheckForUpdates();
        LOG_VERBOSE_OR_DEBUG << "switcher telemetry:" << m_Switcher.getUpdateEnabled();
        if (getIsTelemetryEnabled()) {
            doStartReporting();
        } else {
            LOG_WARNING << "Telemetry is disabled";
        }
    }

    void TelemetryService::stopReporting(bool immediately) {
        LOG_DEBUG << "#";
        if (isRunning()) {
            m_TelemetryWorker->stopWorking(immediately);
        } else {
            LOG_WARNING << "Service is not running";
        }

        m_IsStopped = true;
    }

    void TelemetryService::handleMessage(Common::NamedType<UserAction> const &event) {
        reportAction(event.get());
    }

    bool TelemetryService::isRunning() {
        return m_TelemetryWorker != nullptr && !m_IsStopped;
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

        m_TelemetryWorker = new TelemetryWorker(m_UserAgentId,
                                                m_ReportingEndpoint,
                                                m_InterfaceLanguage,
                                                m_SettingsModel);

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
        return m_SettingsModel.getIsTelemetryEnabled() && m_Switcher.getIsTelemetryEnabled();
    }

    void TelemetryService::doReportAction(UserAction action) {
        if (isRunning()) {
            auto item = std::make_shared<AnalyticsUserEvent>(action);
            m_TelemetryWorker->submitItem(item);
        } else {
            LOG_WARNING << "Service is not running";
        }
    }

    void TelemetryService::reportAction(UserAction action) {
        if (getIsTelemetryEnabled()) {
            doReportAction(action);
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

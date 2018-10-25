/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "requestsservice.h"

#include <QThread>
#include <QtDebug>
#include <QtGlobal>

#include "Common/logging.h"
#include "Connectivity/configrequest.h"
#include "Connectivity/requestsworker.h"
#include "Helpers/remoteconfig.h"

#define NO_CACHE_ATTRIBUTE true

namespace Connectivity {
    class IConnectivityRequest;

    RequestsService::RequestsService(const Models::ProxySettings &proxySettings, QObject *parent):
        QObject(parent),
        m_RequestsWorker(nullptr),
        m_ProxySettings(proxySettings)
    {
    }

    void RequestsService::startService() {
        Q_ASSERT(m_RequestsWorker == nullptr);
        LOG_DEBUG << "#";

        m_RequestsWorker = new RequestsWorker(m_ProxySettings);

        QThread *thread = new QThread();
        m_RequestsWorker->moveToThread(thread);

        QObject::connect(thread, &QThread::started, m_RequestsWorker, &RequestsWorker::process);
        QObject::connect(m_RequestsWorker, &RequestsWorker::stopped, thread, &QThread::quit);

        QObject::connect(m_RequestsWorker, &RequestsWorker::stopped, m_RequestsWorker, &RequestsWorker::deleteLater);
        QObject::connect(thread, &QThread::finished, thread, &QThread::deleteLater);

        QObject::connect(this, &RequestsService::cancelServing,
                         m_RequestsWorker, &RequestsWorker::cancel);

        QObject::connect(m_RequestsWorker, &RequestsWorker::stopped,
                         this, &RequestsService::workerFinished);
        QObject::connect(m_RequestsWorker, &RequestsWorker::destroyed,
                         this, &RequestsService::workerDestroyed);

        thread->start(/*QThread::LowPriority*/);
    }

    void RequestsService::stopService() {
        LOG_DEBUG << "#";
        Q_ASSERT(m_RequestsWorker != nullptr);
        m_RequestsWorker->stopWorking();
    }

    void RequestsService::receiveConfig(Helpers::RemoteConfig &config) {
        LOG_DEBUG << "#";
        if (m_RequestsWorker == nullptr) {
            LOG_DEBUG << "Skipping" << config.getUrl() << ". Service is stopped";
            return;
        }

        auto item = std::make_shared<ConfigRequest>(config, config.getUrl(), NO_CACHE_ATTRIBUTE);
        m_RequestsWorker->submitItem(item);
    }

    void RequestsService::sendRequest(const std::shared_ptr<IConnectivityRequest> &request) {
        LOG_DEBUG << "#";

        if (m_RequestsWorker == nullptr) {
            LOG_DEBUG << "Skipping request. Service is stopped";
            return;
        }

        m_RequestsWorker->submitItem(request);
    }

    void RequestsService::sendRequestSync(std::shared_ptr<IConnectivityRequest> &request) {
        LOG_DEBUG << "#";

        if (m_RequestsWorker == nullptr) {
            LOG_DEBUG << "Skipping request. Service is stopped";
            return;
        }

        m_RequestsWorker->sendRequestSync(request);
    }

    void RequestsService::workerFinished() {
        LOG_DEBUG << "#";
    }

    void RequestsService::workerDestroyed(QObject *object) {
        LOG_DEBUG << "#";
        Q_UNUSED(object);
        m_RequestsWorker = nullptr;
    }
}

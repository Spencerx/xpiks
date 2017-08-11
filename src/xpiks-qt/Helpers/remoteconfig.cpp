/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2017 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "remoteconfig.h"
#include <QThread>
#include "../Common/defines.h"
#include "../Conectivity/simplecurlrequest.h"
#include "../Models/proxysettings.h"

namespace Helpers {
    RemoteConfig::RemoteConfig(QObject *parent):
        QObject(parent)
    {
    }

    RemoteConfig::~RemoteConfig() {
    }

    void RemoteConfig::requestInitConfig(const QString &configUrl, Models::ProxySettings *proxySettings) {
        m_ConfigUrl = configUrl;
        LOG_DEBUG << m_ConfigUrl;

        Conectivity::SimpleCurlRequest *request = new Conectivity::SimpleCurlRequest(configUrl);
        request->setProxySettings(proxySettings);

        QThread *thread = new QThread();

        request->moveToThread(thread);

        QObject::connect(thread, SIGNAL(started()), request, SLOT(process()));
        QObject::connect(request, SIGNAL(stopped()), thread, SLOT(quit()));

        QObject::connect(request, SIGNAL(stopped()), request, SLOT(deleteLater()));
        QObject::connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));

        QObject::connect(request, SIGNAL(requestFinished(bool)), this, SLOT(requestFinishedHandler(bool)));

        thread->start(QThread::LowPriority);
        LOG_INFO << "Started request thread for" << configUrl;
    }

    void RemoteConfig::requestFinishedHandler(bool success) {
        LOG_INFO << "success:" << success;

        Conectivity::SimpleCurlRequest *request = qobject_cast<Conectivity::SimpleCurlRequest *>(sender());

        if (success) {
            QJsonParseError error;

            auto responseData = request->getResponseData();
            LOG_INTEGR_TESTS_OR_DEBUG << responseData;

            m_Config = QJsonDocument::fromJson(responseData, &error);

            if (error.error == QJsonParseError::NoError) {
                emit configArrived();
            } else {
                LOG_INTEGRATION_TESTS << m_ConfigUrl;
                LOG_WARNING << "Failed to parse remote json" << error.errorString();
            }
        }

        request->dispose();
    }
}

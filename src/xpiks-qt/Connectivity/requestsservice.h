/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef REQUESTSSERVICE_H
#define REQUESTSSERVICE_H

#include <QObject>
#include <memory>
#include "../Common/baseentity.h"
#include "iconnectivityrequest.h"

namespace Helpers {
    class RemoteConfig;
}

namespace Models {
    class ProxySettings;
}

namespace Connectivity {
    class RequestsWorker;

    class RequestsService : public QObject
    {
        Q_OBJECT
    public:
        explicit RequestsService(Models::ProxySettings *proxySettings, QObject *parent = 0);

    public:
        void startService();
        void stopService();

    public:
        void receiveConfig(Helpers::RemoteConfig *config);
        void sendRequest(const std::shared_ptr<IConnectivityRequest> &request);
        void sendRequestSync(std::shared_ptr<IConnectivityRequest> &request);

    signals:
        void cancelServing();

    private slots:
        void workerFinished();
        void workerDestroyed(QObject *object);

    private:
        RequestsWorker *m_RequestsWorker;
        Models::ProxySettings *m_ProxySettings;
    };
}

#endif // REQUESTSSERVICE_H

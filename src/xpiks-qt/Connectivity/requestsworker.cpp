/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "requestsworker.h"
#include "simplecurlrequest.h"
#include "../Models/proxysettings.h"

namespace Connectivity {
    RequestsWorker::RequestsWorker(Models::ProxySettings *proxySettings, QObject *parent) :
        QObject(parent),
        m_ProxySettings(proxySettings)
    {
        Q_ASSERT(proxySettings != nullptr);
    }

    void RequestsWorker::sendRequestSync(std::shared_ptr<IConnectivityRequest> &item) {
        LOG_DEBUG << "#";
        sendRequest(item);
    }

    bool RequestsWorker::initWorker() {
        LOG_DEBUG << "#";
        return true;
    }

    void RequestsWorker::sendRequest(std::shared_ptr<IConnectivityRequest> &item) {
        auto url = item->getResourceURL();
        LOG_INFO << "Request:" << url;

        SimpleCurlRequest request(url);
        request.setProxySettings(m_ProxySettings);
        request.addRawHeaders(item->getRawHeaders());

        Common::flag_t flags = item->getFlags();
        if (Common::HasFlag(flags, IConnectivityRequest::NoCache)) {
            request.addRawHeaders(QStringList() << "Cache-Control: no-cache");
        }

        const bool success = request.sendRequestSync();
        if (!success) { LOG_WARNING << "Failed to process" << url; }

        IConnectivityResponse *response = item->getResponse();
        if (response != nullptr) {
            response->setResult(success, request.getResponseData());
        }
    }
}

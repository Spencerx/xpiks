/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "requestsworker.h"

#include <QStringList>
#include <QtDebug>

#include "Common/flags.h"
#include "Common/logging.h"
#include "Common/types.h"
#include "Connectivity/iconnectivityrequest.h"
#include "Connectivity/iconnectivityresponse.h"
#include "Connectivity/simplecurlrequest.h"
#include "Models/settingsmodel.h"
#include "Models/Connectivity/proxysettings.h"

namespace Connectivity {
    RequestsWorker::RequestsWorker(Models::SettingsModel &settingsModel, QObject *parent) :
        QObject(parent),
        m_SettingsModel(settingsModel)
    {
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
        Models::ProxySettings *proxySettings = m_SettingsModel.retrieveProxySettings();
        request.setProxySettings(proxySettings);
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

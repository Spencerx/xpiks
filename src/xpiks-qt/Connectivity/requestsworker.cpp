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
#include <QThread>

#include "Common/flags.h"
#include "Common/logging.h"
#include "Common/types.h"
#include "Connectivity/iconnectivityrequest.h"
#include "Connectivity/iconnectivityresponse.h"
#include "Connectivity/simplecurlrequest.h"
#include "Models/settingsmodel.h"
#include "Models/Connectivity/proxysettings.h"

#define RETRY_SLEEP_MILLIS 500

namespace Connectivity {
    void prepareRequest(SimpleCurlRequest &request,
                        const std::shared_ptr<IConnectivityRequest> &item,
                        Models::ProxySettings *proxySettings) {
        request.setProxySettings(proxySettings);
        request.addRawHeaders(item->getRawHeaders());

        Common::flag_t flags = item->getFlags();
        if (Common::HasFlag(flags, IConnectivityRequest::NoCache)) {
            request.addRawHeaders(QStringList() << "Cache-Control: no-cache");
        }
    }

    RequestsWorker::RequestsWorker(Models::SettingsModel &settingsModel, QObject *parent) :
        QObject(parent),
        m_SettingsModel(settingsModel)
    {
    }

    void RequestsWorker::sendRequestSync(const std::shared_ptr<IConnectivityRequest> &item) {
        LOG_DEBUG << "#";
        auto url = item->getResourceURL();
        LOG_INFO << "Request:" << url;

        SimpleCurlRequest request(url);
        prepareRequest(request, item, m_SettingsModel.retrieveProxySettings());

        bool success = request.sendRequestSync();

        if (!success && !Common::HasFlag(item->getFlags(), IConnectivityRequest::Retry)) {
            LOG_WARNING << "Request" << url << "failed, retrying 1 time";
            {
                LOG_VERBOSE_OR_DEBUG << "Sleeping before retry";
                QThread::msleep(RETRY_SLEEP_MILLIS);
            }
            success = request.sendRequestSync();
        }

        IConnectivityResponse *response = item->getResponse();
        if (response != nullptr) {
            response->setResult(success, request.getResponseData());
        }
    }

    bool RequestsWorker::initWorker() {
        LOG_DEBUG << "#";
        return true;
    }

    std::shared_ptr<void> RequestsWorker::processWorkItem(WorkItem &workItem) {
        auto &item = workItem.m_Item;
        auto url = item->getResourceURL();
        LOG_INFO << "Request:" << url;

        SimpleCurlRequest request(url);
        prepareRequest(request, item, m_SettingsModel.retrieveProxySettings());

        const bool success = request.sendRequestSync();

        if (!success && !Common::HasFlag(item->getFlags(), IConnectivityRequest::Retry)) {
            LOG_WARNING << "Request" << url << "failed, retrying 1 time";
            item->setFlag(IConnectivityRequest::Retry);
            batch_id_t batchId = this->submitItem(item);
            if (batchId - workItem.m_ID == 1) {
                LOG_VERBOSE_OR_DEBUG << "No items in the queue, sleeping before retry";
                QThread::msleep(RETRY_SLEEP_MILLIS);
            }
        } else {
            IConnectivityResponse *response = item->getResponse();
            if (response != nullptr) {
                response->setResult(success, request.getResponseData());
            }
        }

        return std::shared_ptr<void>();
    }
}

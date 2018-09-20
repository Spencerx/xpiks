/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef REQUESTSWORKER_H
#define REQUESTSWORKER_H

#include <QObject>
#include "iconnectivityrequest.h"
#include <Common/itemprocessingworker.h>

namespace Models {
    class ProxySettings;
}

namespace Connectivity {
    class RequestsWorker: public QObject, public Common::ItemProcessingWorker<IConnectivityRequest>
    {
        Q_OBJECT
    public:
        explicit RequestsWorker(const Models::ProxySettings &proxySettings, QObject *parent = 0);

    public:
        void sendRequestSync(std::shared_ptr<IConnectivityRequest> &item);

    protected:
        virtual bool initWorker() override;
        virtual void processOneItem(std::shared_ptr<IConnectivityRequest> &item) override { sendRequest(item); }

    private:
        void sendRequest(std::shared_ptr<IConnectivityRequest> &item);

    protected:
        virtual void onQueueIsEmpty() override { emit queueIsEmpty(); }
        virtual void onWorkerStopped() override { emit stopped(); }

    public slots:
        void process() { doWork(); }
        void cancel() { stopWorking(); }

    signals:
        void stopped();
        void queueIsEmpty();

    private:
        const Models::ProxySettings &m_ProxySettings;
    };
}

#endif // REQUESTSWORKER_H

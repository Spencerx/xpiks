/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SANDBOXEDDEPENDENCIES_H
#define SANDBOXEDDEPENDENCIES_H

#include <memory>

#include <QHash>
#include <QString>

#include "Plugins/iuiprovider.h"
#include "Microstocks/imicrostockservices.h"
#include "Microstocks/microstockenums.h"

class QObject;
class QUrl;

namespace Connectivity {
    class RequestsService;
}

namespace Microstocks {
    class IMicrostockAPIClients;
    class IMicrostockService;
}

namespace Plugins {
    class UIProvider;

    class UIProviderSafe : public IUIProvider {
    public:
        UIProviderSafe(int pluginID, UIProvider &realUIProvider);

    public:
        virtual void openDialog(const QUrl &rcPath, const QHash<QString, QObject*> &contextModels = QHash<QString, QObject*>()) const override;
        virtual int addTab(const QString &tabIconUrl, const QString &tabComponentUrl, QObject *tabModel) const override;
        virtual bool removeTab(int tabID) const override;

    private:
        int m_PluginID;
        UIProvider &m_RealUIProvider;
    };

    class MicrostockServicesSafe: public Microstocks::IMicrostockServices {
    public:
        MicrostockServicesSafe(Microstocks::IMicrostockAPIClients &apiClients,
                               Connectivity::RequestsService &requestsService);

        // IMicrostockServices interface
    public:
        virtual std::shared_ptr<Microstocks::IMicrostockService> getService(Microstocks::MicrostockType type) override;

    private:
        Microstocks::IMicrostockAPIClients &m_ApiClients;
        Connectivity::RequestsService &m_RequestsService;
    };
}

#endif // SANDBOXEDDEPENDENCIES_H

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

#include "../Plugins/iuiprovider.h"
#include "../Microstocks/imicrostockservices.h"
#include "../Microstocks/microstockservice.h"
#include "../Connectivity/requestsservice.h"
#include "../Microstocks/microstockapiclients.h"

namespace Models {
    class UIManager;
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
        MicrostockServicesSafe(Connectivity::RequestsService &requestsService,
                               Microstocks::MicrostockAPIClients &apiClients);

        // IMicrostockServices interface
    public:
        virtual Microstocks::IMicrostockService *getShutterstockService() override { return &m_ShutterstockService; }
        virtual Microstocks::IMicrostockService *getFotoliaService() override { return &m_FotoliaService; }
        virtual Microstocks::IMicrostockService *getGettyService() override { return &m_GettyService; }

    private:
        Microstocks::MicrostockService m_ShutterstockService;
        Microstocks::MicrostockService m_FotoliaService;
        Microstocks::MicrostockService m_GettyService;
    };
}

#endif // SANDBOXEDDEPENDENCIES_H

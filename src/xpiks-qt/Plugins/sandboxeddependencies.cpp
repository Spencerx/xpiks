/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "sandboxeddependencies.h"

#include "Microstocks/imicrostockapiclients.h"
#include "Microstocks/microstockservice.h"
#include "Models/uimanager.h"
#include "Plugins/uiprovider.h"

namespace Plugins {
    UIProviderSafe::UIProviderSafe(int pluginID, UIProvider &realUIProvider):
        m_PluginID(pluginID),
        m_RealUIProvider(realUIProvider)
    {
    }

    void UIProviderSafe::openDialog(const QUrl &rcPath, const QHash<QString, QObject *> &contextModels) const {
        m_RealUIProvider.openDialog(rcPath, contextModels);
    }

    int UIProviderSafe::addTab(const QString &tabIconUrl, const QString &tabComponentUrl, QObject *tabModel) const {
        auto &uiManager = m_RealUIProvider.getUIManager();
        int result = uiManager.addPluginTab(m_PluginID, tabIconUrl, tabComponentUrl, tabModel);
        return result;
    }

    bool UIProviderSafe::removeTab(int tabID) const {
        auto &uiManager = m_RealUIProvider.getUIManager();
        bool result = uiManager.removePluginTab(m_PluginID, tabID);
        return result;
    }

    MicrostockServicesSafe::MicrostockServicesSafe(Microstocks::IMicrostockAPIClients &apiClients,
                                                   Connectivity::RequestsService &requestsService):
        m_ApiClients(apiClients),
        m_RequestsService(requestsService)
    {
    }

    std::shared_ptr<Microstocks::IMicrostockService> MicrostockServicesSafe::getService(Microstocks::MicrostockType type) {
        return std::make_shared<Microstocks::MicrostockService>(
                    m_ApiClients.getClient(type),
                    m_RequestsService);
    }
}

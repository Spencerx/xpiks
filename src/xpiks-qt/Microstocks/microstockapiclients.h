/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef MICROSTOCKAPICLIENTS_H
#define MICROSTOCKAPICLIENTS_H

#include <map>
#include <memory>

#include "Microstocks/imicrostockapiclients.h"
#include "Microstocks/microstockenums.h"
#include "Microstocks/imicrostockapiclient.h"

namespace Connectivity {
    class IConnectivityResponse;
}

namespace Microstocks {
//    class IMicrostockAPIClient;
    class SearchQuery;

    class MicrostockAPIClients: public IMicrostockAPIClients
    {
    public:
        MicrostockAPIClients() {}

    public:
        void addClient(std::shared_ptr<IMicrostockAPIClient> const &client) {
            Q_ASSERT(m_ClientMap.find((int)client->type()) == m_ClientMap.end());
            m_ClientMap[(int)client->type()] = client;
        }

        // IMicrostockServices interface
    public:
        virtual std::shared_ptr<IMicrostockAPIClient> getClient(MicrostockType type) override {
            std::shared_ptr<IMicrostockAPIClient> result;
            auto it = m_ClientMap.find((int)type);
            if (it != m_ClientMap.end()) {
                result = it->second;
            } else {
                Q_ASSERT(false);
            }
            return result;
        }

    private:
        std::map<int, std::shared_ptr<IMicrostockAPIClient>> m_ClientMap;
    };
}

#endif // MICROSTOCKAPICLIENTS_H

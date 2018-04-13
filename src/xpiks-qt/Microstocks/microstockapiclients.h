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

#include "shutterstockapiclient.h"
#include "fotoliaapiclient.h"
#include "gettyapiclient.h"

namespace Microstocks {
    class MicrostockAPIClients
    {
    public:
        ShutterstockAPIClient &getShutterstockClient() { return m_ShutterstockClient; }
        FotoliaAPIClient &getFotoliaClient() { return m_FotoliaClient; }
        GettyAPIClient &getGettyClient() { return m_GettyClient; }

    private:
        ShutterstockAPIClient m_ShutterstockClient;
        FotoliaAPIClient m_FotoliaClient;
        GettyAPIClient m_GettyClient;
    };
}

#endif // MICROSTOCKAPICLIENTS_H

/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef MICROSTOCKSERVICE_H
#define MICROSTOCKSERVICE_H

#include "imicrostockservice.h"
#include "imicrostockapiclient.h"
#include "../Connectivity/requestsservice.h"

namespace Microstocks {
    class MicrostockService: public IMicrostockService {
    public:
        MicrostockService(IMicrostockAPIClient *apiClient, Connectivity::RequestsService &requestsService);

        // IMicrostockService interface
    public:
        virtual void search(const SearchQuery &query, const std::shared_ptr<Connectivity::IConnectivityResponse> &response) override;

    private:
        IMicrostockAPIClient *m_ApiClient;
        Connectivity::RequestsService &m_RequestsService;
    };
}

#endif // MICROSTOCKSERVICE_H

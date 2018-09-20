/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef IMICROSTOCKAPICLIENT_H
#define IMICROSTOCKAPICLIENT_H

#include <memory>
#include "microstockenums.h"

namespace Connectivity {
    class IConnectivityRequest;
    class IConnectivityResponse;
}

namespace Microstocks {
    class SearchQuery;

    class IMicrostockAPIClient {
    public:
        virtual ~IMicrostockAPIClient() {}
        virtual MicrostockType type() const = 0;
        virtual std::shared_ptr<Connectivity::IConnectivityRequest> search(const SearchQuery &query,
                                                                           const std::shared_ptr<Connectivity::IConnectivityResponse> &response) = 0;
    };
}

#endif // IMICROSTOCKAPICLIENT_H

/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef IMICROSTOCKSERVICE_H
#define IMICROSTOCKSERVICE_H

#include <memory>

namespace Connectivity {
    class IConnectivityResponse;
}

namespace Microstocks {
    class SearchQuery;

    class IMicrostockService {
    public:
        virtual ~IMicrostockService() {}

        virtual void search(SearchQuery const &query,
                            std::shared_ptr<Connectivity::IConnectivityResponse> const &response) = 0;
    };
}

#endif // IMICROSTOCKSERVICE_H

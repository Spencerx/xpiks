/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SHUTTERSTOCKAPICLIENT_H
#define SHUTTERSTOCKAPICLIENT_H

#include <QString>
#include <QUrl>
#include "imicrostockapiclient.h"

namespace Encryption {
    class ISecretsStorage;
}

namespace Microstocks {
    class ShutterstockAPIClient: public IMicrostockAPIClient
    {
    public:
        ShutterstockAPIClient(std::shared_ptr<Encryption::ISecretsStorage> const &secretsStorage);

        // IMicrostockAPIClient interface
    public:
        virtual MicrostockType type() const override { return MicrostockType::Shutterstock; }
        virtual std::shared_ptr<Connectivity::IConnectivityRequest> search(const SearchQuery &query,
                                                                           const std::shared_ptr<Connectivity::IConnectivityResponse> &response) override;

    private:
        QUrl buildSearchQuery(const SearchQuery &query) const;
        QString resultsTypeToString(const SearchQuery &query) const;
        QString orderingToString(const SearchQuery &query) const;

    private:
        std::shared_ptr<Encryption::ISecretsStorage> m_SecretsStorage;
    };
}

#endif // SHUTTERSTOCKAPICLIENT_H

/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "shutterstockapiclient.h"
#include <QUrlQuery>
#include "../Common/defines.h"
#include "../Encryption/aes-qt.h"
#include "../Connectivity/simpleapirequest.h"
#include "apisecrets.h"

namespace Microstocks {
    ShutterstockAPIClient::ShutterstockAPIClient(Encryption::ISecretsStorage *secretsStorage):
        m_SecretsStorage(secretsStorage)
    {
        Q_ASSERT(secretsStorage != nullptr);
    }

    std::shared_ptr<Connectivity::IConnectivityRequest> ShutterstockAPIClient::search(const SearchQuery &query, const std::shared_ptr<Connectivity::IConnectivityResponse> &response) {
        LOG_INFO << query.m_SearchTerms;
        QUrl url = buildSearchQuery(query);

        Encryption::SecretPair clientID, clientSecret;
        if (!m_SecretsStorage->tryFindPair(ShutterstockClientID, clientID)) { Q_ASSERT(false); }
        if (!m_SecretsStorage->tryFindPair(ShutterstockClientSecret, clientSecret)) { Q_ASSERT(false); }

        QString decodedClientId = Encryption::decodeText(clientID.m_Value, clientID.m_Key);
        QString decodedClientSecret = Encryption::decodeText(clientSecret.m_Value, clientSecret.m_Key);

        QString authStr = QString("%1:%2").arg(decodedClientId).arg(decodedClientSecret);
        QString headerData = "Authorization: Basic " + QString::fromLatin1(authStr.toLocal8Bit().toBase64());

        QString resourceUrl = QString::fromLocal8Bit(url.toEncoded());

        std::shared_ptr<Connectivity::IConnectivityRequest> request(
                    new Connectivity::SimpleAPIRequest(resourceUrl, QStringList() << headerData, response));
        return request;
    }

    QUrl ShutterstockAPIClient::buildSearchQuery(const SearchQuery &query) const {
        QUrlQuery urlQuery;

        urlQuery.addQueryItem("language", "en");
        urlQuery.addQueryItem("view", "full");
        urlQuery.addQueryItem("per_page", QString::number(query.m_PageSize));
        urlQuery.addQueryItem("sort", "popular");
        urlQuery.addQueryItem("query", query.m_SearchTerms.join(' '));

        if (!Common::HasFlag(query.m_Flags, Microstocks::AllImages)) {
            urlQuery.addQueryItem("image_type", resultsTypeToString(query.m_Flags));
        }

        QUrl url;
        url.setUrl(QLatin1String("https://api.shutterstock.com/v2/images/search"));
        url.setQuery(urlQuery);
        return url;
    }

    QString ShutterstockAPIClient::resultsTypeToString(Common::flag_t queryFlags) const {
        if (Common::HasFlag(queryFlags, Microstocks::Photos)) { return QLatin1String("photo"); }
        else if (Common::HasFlag(queryFlags, Microstocks::Vectors)) { return QLatin1String("vector"); }
        else if (Common::HasFlag(queryFlags, Microstocks::Illustrations)) { return QLatin1String("illustration"); }
        else { return QString(); }
    }
}

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

namespace Microstocks {
    ShutterstockAPIClient::ShutterstockAPIClient()
    {
        m_ClientId = "28a2a9b917961a0cbc343c81b2dd0f6618377f9210aa3182e5cc9f5588f914d918ede1533c9e06b91769c89e80909743";
        m_ClientSecret = "5092d9a967c2f19b57aac29bc09ac3b9e6ae5baec1a371331b73ff24f1625d95c4f3fef90bdacfbe9b0b3803b48c269192bc55f14bb9c2b5a16d650cd641b746eb384fcf9dbd53a96f1f81215921b04409f3635ecf846ffdf01ee04ba76624c9";
    }

    std::shared_ptr<Connectivity::IConnectivityRequest> ShutterstockAPIClient::search(const SearchQuery &query, const std::shared_ptr<Connectivity::IConnectivityResponse> &response) {
        LOG_INFO << query.m_SearchTerms;
        QUrl url = buildSearchQuery(query);

        QString decodedClientId = Encryption::decodeText(m_ClientId, "MasterPassword");
        QString decodedClientSecret = Encryption::decodeText(m_ClientSecret, "MasterPassword");

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

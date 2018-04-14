/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "gettyapiclient.h"
#include <QUrlQuery>
#include "../Encryption/aes-qt.h"
#include "../Connectivity/simpleapirequest.h"
#include "apisecrets.h"

namespace Microstocks {
    GettyAPIClient::GettyAPIClient(Encryption::ISecretsStorage *secretsStorage):
        m_SecretsStorage(secretsStorage)
    {
        Q_ASSERT(secretsStorage != nullptr);
    }

    std::shared_ptr<Connectivity::IConnectivityRequest> GettyAPIClient::search(const SearchQuery &query, const std::shared_ptr<Connectivity::IConnectivityResponse> &response) {
        QUrl url = buildSearchQuery(query);
        QString resourceUrl = QString::fromLocal8Bit(url.toEncoded());

        Encryption::SecretPair apiSecret;
        if (!m_SecretsStorage->tryFindPair(GettyAPIKey, apiSecret)) { Q_ASSERT(false); }

        QString decodedAPIKey = Encryption::decodeText(apiSecret.m_Value, apiSecret.m_Key);
        QString headerData = "Api-Key: " + decodedAPIKey;

        std::shared_ptr<Connectivity::IConnectivityRequest> request(new Connectivity::SimpleAPIRequest(resourceUrl, QStringList() << headerData, response));
        return request;
    }

    QUrl GettyAPIClient::buildSearchQuery(const SearchQuery &query) const {
        QUrlQuery urlQuery;

        urlQuery.addQueryItem("fields", "keywords,preview,title,id,caption");
        urlQuery.addQueryItem("phrase", query.m_SearchTerms.join(' '));
        urlQuery.addQueryItem("page", "1");
        urlQuery.addQueryItem("page_size", QString::number(query.m_PageSize));
        urlQuery.addQueryItem("sort_order", "most_popular");

        if (!Common::HasFlag(query.m_Flags, Microstocks::AllImages)) {
            urlQuery.addQueryItem("graphical_styles", resultsTypeToString(query.m_Flags));
        }

        QUrl url;
        url.setUrl(QLatin1String("https://api.gettyimages.com:443/v3/search/images"));
        url.setQuery(urlQuery);
        return url;
    }

    QString GettyAPIClient::resultsTypeToString(Common::flag_t queryFlags) const {
        if (Common::HasFlag(queryFlags, Microstocks::Photos)) { return QLatin1String("photography"); }
        else if (Common::HasFlag(queryFlags, Microstocks::Vectors)) { return QLatin1String("illustration"); }
        else if (Common::HasFlag(queryFlags, Microstocks::Illustrations)) { return QLatin1String("fine_art"); }
        else { return QString(); }
    }
}

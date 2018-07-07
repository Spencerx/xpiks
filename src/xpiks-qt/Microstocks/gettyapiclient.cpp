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

        auto request = std::make_shared<Connectivity::SimpleAPIRequest>(
                           resourceUrl, QStringList() << headerData, response);
        return request;
    }

    QUrl GettyAPIClient::buildSearchQuery(const SearchQuery &query) const {
        QUrlQuery urlQuery;
        // http://developers.gettyimages.com/api/

        urlQuery.addQueryItem("fields", "keywords,preview,title,id,caption");
        urlQuery.addQueryItem("phrase", query.getSearchQuery());
        urlQuery.addQueryItem("page", QString::number(query.getPageIndex() + 1));
        urlQuery.addQueryItem("page_size", QString::number(query.getPageSize()));
        urlQuery.addQueryItem("sort_order", orderingToString(query));

        if (!query.getSearchAllImages()) {
            urlQuery.addQueryItem("graphical_styles", resultsTypeToString(query));
        }

        QUrl url;
        url.setUrl(QLatin1String("https://api.gettyimages.com:443/v3/search/images"));
        url.setQuery(urlQuery);
        return url;
    }

    QString GettyAPIClient::resultsTypeToString(const SearchQuery &query) const {
        if (query.getSearchPhotos()) { return QLatin1String("photography"); }
        else if (query.getSearchVectors()) { return QLatin1String("illustration"); }
        else if (query.getSearchIllustrations()) { return QLatin1String("fine_art"); }
        else { return QString(); }
    }

    QString GettyAPIClient::orderingToString(const SearchQuery &query) const {
        // http://developers.gettyimages.com/docs/#operation/Search_GetCreativeImagesByPhrase
        if (query.getOrderNewest()) { return QLatin1String("newest"); }
        else if (query.getOrderRelevance()) { return QLatin1String("best_match"); }
        else if (query.getOrderPopular()) { return QLatin1String("most_popular"); }
        else { return QString(); }
    }
}

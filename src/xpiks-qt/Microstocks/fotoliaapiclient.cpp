/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "fotoliaapiclient.h"
#include <QUrlQuery>
#include "../Connectivity/simpleapirequest.h"
#include "../Encryption/aes-qt.h"
#include "apisecrets.h"

Microstocks::FotoliaAPIClient::FotoliaAPIClient(Encryption::ISecretsStorage *secretsStorage):
    m_SecretsStorage(secretsStorage)
{
    Q_ASSERT(secretsStorage != nullptr);
}

std::shared_ptr<Connectivity::IConnectivityRequest> Microstocks::FotoliaAPIClient::search(const Microstocks::SearchQuery &query, const std::shared_ptr<Connectivity::IConnectivityResponse> &response) {
    Encryption::SecretPair apiSecret;
    if (!m_SecretsStorage->tryFindPair(FotoliaAPIKey, apiSecret)) { Q_ASSERT(false); }

    QUrl url = buildSearchQuery(query);
    QString resourceUrl = QString::fromLocal8Bit(url.toEncoded());

    QString decodedAPIKey = Encryption::decodeText(apiSecret.m_Value, apiSecret.m_Key);
    QString authStr = QString("%1:").arg(decodedAPIKey);
    QString headerData = "Authorization: Basic " + QString::fromLatin1(authStr.toLocal8Bit().toBase64());

    std::shared_ptr<Connectivity::IConnectivityRequest> request(
                new Connectivity::SimpleAPIRequest(resourceUrl, QStringList() << headerData, response));
    return request;
}

QUrl Microstocks::FotoliaAPIClient::buildSearchQuery(const Microstocks::SearchQuery &query) const {
    QUrlQuery urlQuery;

    urlQuery.addQueryItem("search_parameters[language_id]", "2");
    urlQuery.addQueryItem("search_parameters[thumbnail_size]", "160");
    urlQuery.addQueryItem("search_parameters[limit]", QString::number(query.getPageSize()));
    urlQuery.addQueryItem("search_parameters[offset]", QString::number(query.getPageSize() * query.getPageIndex()));
    urlQuery.addQueryItem("search_parameters[order]", orderingToString(query));
    urlQuery.addQueryItem("search_parameters[words]", query.getSearchQuery());
    urlQuery.addQueryItem("result_columns[0]", "nb_results");
    urlQuery.addQueryItem("result_columns[1]", "title");
    urlQuery.addQueryItem("result_columns[2]", "keywords");
    urlQuery.addQueryItem("result_columns[3]", "thumbnail_url");
    urlQuery.addQueryItem("result_columns[4]", "id");
    urlQuery.addQueryItem(resultsTypeToString(query), "1");

    QUrl url;
    url.setUrl(QLatin1String("https://api.fotolia.com/Rest/1/search/getSearchResults"));
    //url.setPassword("");
    url.setQuery(urlQuery);
    return url;
}

QString Microstocks::FotoliaAPIClient::resultsTypeToString(const SearchQuery &query) const {
    if (query.getSearchAllImages()) { return QLatin1String("search_parameters[filters][content_type:all]"); }
    else if (query.getSearchPhotos()) { return QLatin1String("search_parameters[filters][content_type:photo]"); }
    else if (query.getSearchVectors()) { return QLatin1String("search_parameters[filters][content_type:vector]"); }
    else if (query.getSearchIllustrations()) { return QLatin1String("search_parameters[filters][content_type:illustration]"); }
    else { return QString(); }
}

QString Microstocks::FotoliaAPIClient::orderingToString(const Microstocks::SearchQuery &query) const {
    /*
     * relevance | price_1 | creation | nb_views | nb_downloads
     * Relevance | price ASC | creation date DESC | number of views DESC | number of downloads DESC
    */
    if (query.getOrderNewest()) { return QLatin1String("creation"); }
    else if (query.getOrderRelevance()) { return QLatin1String("relevance"); }
    else if (query.getOrderPopular()) { return QLatin1String("nb_downloads"); }
    else { return QString(); }
}

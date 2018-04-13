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

namespace Microstocks {
    GettyAPIClient::GettyAPIClient()
    {
        m_GettyImagesAPIKey = QLatin1String("17a45639c3bf88f7a6d549759af398090c3f420e53a61a06d7a2a2b153c89fc9470b2365dae8c6d92203287dc6f69f55b230835a8fb2a70b24e806771b750690");
    }

    std::shared_ptr<Connectivity::IConnectivityRequest> GettyAPIClient::search(const SearchQuery &query, const std::shared_ptr<Connectivity::IConnectivityResponse> &response) {
        QUrl url = buildSearchQuery(query);
        QString resourceUrl = QString::fromLocal8Bit(url.toEncoded());

        QString decodedAPIKey = Encryption::decodeText(m_GettyImagesAPIKey, "MasterPassword");
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

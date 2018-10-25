/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "fotoliasuggestionengine.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>
#include <QJsonValueRef>
#include <QStringList>
#include <QtDebug>

#include "Common/logging.h"
#include "Suggestion/suggestionartwork.h"

namespace Suggestion {
    void parseJsonResults(const QJsonObject &jsonObject, int count,
                          std::vector<std::shared_ptr<SuggestionArtwork> > &suggestionArtworks) {
        LOG_DEBUG << "#";
        for (int i = 0; i < count; ++i) {
            QJsonValue item = jsonObject[QString::number(i)];
            if (!item.isObject()) { continue; }
            QJsonObject object = item.toObject();

            QJsonValue url = object["thumbnail_url"];
            if (!url.isString()) { continue; }

            QJsonValue keywords = object["keywords"];
            if (!keywords.isString()) { continue; }

            QJsonValue id = object["id"];
            if (!id.isDouble()) { continue; }
            QString externalUrl = QString("https://fotolia.com/id/%1").arg((int)id.toDouble());

            QJsonValue title = object["title"];
            if (!title.isString()) { continue; }

            QStringList keywordsList = keywords.toString().split(',');

            suggestionArtworks.emplace_back(
                        std::make_shared<SuggestionArtwork>(
                            url.toString(), externalUrl, title.toString(), QString(""), keywordsList));
        }
    }

    bool FotoliaSuggestionResults::parseResponse(const QByteArray &body, std::vector<std::shared_ptr<SuggestionArtwork> > &artworks) {
        bool success = false;
        QJsonParseError error;
        QJsonDocument document = QJsonDocument::fromJson(body, &error);

        if (error.error == QJsonParseError::NoError) {
            QJsonObject jsonObject = document.object();
            QJsonValue nbResultsValue = jsonObject["nb_results"];

            if (!nbResultsValue.isUndefined()) {
                int resultsNumber = nbResultsValue.toInt();
                parseJsonResults(jsonObject, resultsNumber, artworks);
                success = true;
            } else {
                LOG_WARNING << "[nb_results] is undefined";
            }
        } else {
            LOG_WARNING << "parsing error:" << error.errorString();
        }

        return success;
    }
}

/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "gettysuggestionengine.h"
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include "../Common/logging.h"

namespace Suggestion {
    void parseUrl(const QJsonValue &previewObject, QString &url) {
         if (!previewObject.isArray()) { return; }

         QJsonArray previews = previewObject.toArray();
         int size = previews.size();

         if (size > 0) {
             QJsonValue element = previews.first();
             if (element.isObject()) {
                 QJsonObject firstItem = element.toObject();
                 if (firstItem.contains("uri") && firstItem.contains("is_watermarked")) {
                     QJsonValue value = firstItem["uri"];
                     if (value.isString()) {
                         url = value.toString();
                     }
                 }
             }
         }
     }

     void parseKeywords(const QJsonValue &keywordsObject, QStringList &keywords) {
         if (keywordsObject.isArray()) {
             QJsonArray keywordsArray = keywordsObject.toArray();

             int size = keywordsArray.size();
             keywords.reserve(size);

             for (int i = 0; i < size; ++i) {
                 QJsonValue element = keywordsArray.at(i);
                 if (!element.isObject()) { continue; }

                 QJsonObject keywordItemObject = element.toObject();
                 if (keywordItemObject.contains("text")) {
                     QJsonValue textValue = keywordItemObject["text"];
                     if (textValue.isString()) {
                         keywords.append(textValue.toString());
                     }
                 }
             }
         }
     }

    void parseJsonArray(const QJsonObject &jsonObject, int count,
                        std::vector<std::shared_ptr<SuggestionArtwork> > &suggestionArtworks) {
        LOG_DEBUG << "#";
        Q_UNUSED(count);
        if (!jsonObject.contains("images")) { return; }
        QJsonValue imagesValue = jsonObject["images"];

        if (imagesValue.isArray()) {
            QJsonArray imagesArray = imagesValue.toArray();
            int size = imagesArray.size();
            for (int i = 0; i < size; ++i) {
                QJsonValue element = imagesArray.at(i);
                if (!element.isObject()) { continue; }

                QJsonObject item = element.toObject();

                QString url;
                QStringList keywords;

                if (item.contains("display_sizes")) {
                    parseUrl(item["display_sizes"], url);
                }

                if (item.contains("keywords")) {
                    parseKeywords(item["keywords"], keywords);
                }

                QString title;
                QString description;

                if (item.contains("title")) {
                    QJsonValue value = item["title"];
                    if (value.isString()) {
                         title = value.toString();
                    }
                }

                if (item.contains("caption")) {
                    QJsonValue value = item["caption"];
                    if (value.isString()) {
                         description = value.toString();
                    }
                }

                // TODO: parse external url from istock
                suggestionArtworks.emplace_back(new SuggestionArtwork(url, title, description, keywords, false));
            }
        }
    }

    bool GettySuggestionResults::parseResponse(const QByteArray &body, std::vector<std::shared_ptr<SuggestionArtwork> > &artworks) {
        bool success = false;
        QJsonParseError error;

        QJsonDocument document = QJsonDocument::fromJson(body, &error);
        if (error.error == QJsonParseError::NoError) {
            QJsonObject jsonObject = document.object();
            QJsonValue nbResultsValue = jsonObject["result_count"];

            if (!nbResultsValue.isUndefined()) {
                int resultsNumber = nbResultsValue.toInt();
                parseJsonArray(jsonObject, resultsNumber, artworks);
                success = true;
            } else {
                LOG_WARNING << "[result_count] is undefined";
            }
        } else {
            LOG_WARNING << "parsing error:" << error.errorString();
        }

        return success;
    }
}

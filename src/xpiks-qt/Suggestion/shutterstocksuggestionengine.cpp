/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "shutterstocksuggestionengine.h"
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonObject>
#include "../Common/logging.h"

namespace Suggestion {
    void parseJsonResults(const QJsonArray &jsonArray, std::vector<std::shared_ptr<SuggestionArtwork> > &suggestionArtworks) {
        LOG_DEBUG << "#";
        foreach (const QJsonValue &value, jsonArray) {
            QJsonObject imageResult = value.toObject();

            if (imageResult.contains("assets")) {
                QJsonObject assetsObject = imageResult["assets"].toObject();
                if (assetsObject.contains("large_thumb")) {
                    QJsonObject thumb = assetsObject["large_thumb"].toObject();
                    if (thumb.contains("url")) {
                        QString url = thumb["url"].toString();
                        QStringList keywordsList;

                        if (imageResult["keywords"].isArray()) {
                            foreach (const QJsonValue &keyword, imageResult["keywords"].toArray()) {
                                keywordsList.append(keyword.toString());
                            }
                        }

                        QString externalUrl;
                        if (imageResult.contains("id")) {
                            externalUrl = QString("https://www.shutterstock.com/pic-%1.html").arg(imageResult["id"].toString());
                        }

                        QString description;
                        if (imageResult.contains("description")) {
                            description = imageResult["description"].toString();
                        }

                        suggestionArtworks.emplace_back(new SuggestionArtwork(url, externalUrl, QString(""), description, keywordsList));
                    }
                }
            }
        }
    }

    bool ShutterstockSuggestionResults::parseResponse(const QByteArray &body, std::vector<std::shared_ptr<SuggestionArtwork> > &artworks) {
        bool success = false;
        QJsonParseError error;
        QJsonDocument document = QJsonDocument::fromJson(body, &error);

        if (error.error == QJsonParseError::NoError) {
            QJsonObject jsonObject = document.object();
            QJsonValue dataValue = jsonObject["data"];

            if (dataValue.isArray()) {
                parseJsonResults(dataValue.toArray(), artworks);
                success = true;
            } else {
                LOG_WARNING << "[data] element is not an array in" << QString::fromLocal8Bit(document.toJson(QJsonDocument::Indented));
            }
        } else {
            LOG_WARNING << "parsing error:" << error.errorString();
        }

        return success;
    }
}

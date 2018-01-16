/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <QByteArray>
#include <QString>
#include <QThread>
#include "shutterstockqueryengine.h"
#include "suggestionartwork.h"
#include "keywordssuggestor.h"
#include "../Encryption/aes-qt.h"
#include "../Connectivity/simplecurlrequest.h"
#include "../Common/defines.h"
#include "../Models/settingsmodel.h"

namespace Suggestion {
    ShutterstockQueryEngine::ShutterstockQueryEngine(int engineID, Models::SettingsModel *settingsModel):
        SuggestionQueryEngineBase(engineID, settingsModel)
    {
        m_ClientId = "28a2a9b917961a0cbc343c81b2dd0f6618377f9210aa3182e5cc9f5588f914d918ede1533c9e06b91769c89e80909743";
        m_ClientSecret = "5092d9a967c2f19b57aac29bc09ac3b9e6ae5baec1a371331b73ff24f1625d95c4f3fef90bdacfbe9b0b3803b48c269192bc55f14bb9c2b5a16d650cd641b746eb384fcf9dbd53a96f1f81215921b04409f3635ecf846ffdf01ee04ba76624c9";
    }

    void ShutterstockQueryEngine::submitQuery(const SearchQuery &query) {
        LOG_INFO << query.m_SearchTerms;
        QUrl url = buildQuery(query);

        QString decodedClientId = Encryption::decodeText(m_ClientId, "MasterPassword");
        QString decodedClientSecret = Encryption::decodeText(m_ClientSecret, "MasterPassword");

        QString authStr = QString("%1:%2").arg(decodedClientId).arg(decodedClientSecret);
        QString headerData = "Basic " + QString::fromLatin1(authStr.toLocal8Bit().toBase64());

        auto *settings = getSettingsModel();
        auto *proxySettings = settings->retrieveProxySettings();

        QString resourceUrl = QString::fromLocal8Bit(url.toEncoded());
        Connectivity::SimpleCurlRequest *request = new Connectivity::SimpleCurlRequest(resourceUrl);
        request->setRawHeaders(QStringList() << "Authorization: " + headerData);
        request->setProxySettings(proxySettings);

        QThread *thread = new QThread();

        request->moveToThread(thread);

        QObject::connect(thread, &QThread::started, request, &Connectivity::SimpleCurlRequest::process);
        QObject::connect(request, &Connectivity::SimpleCurlRequest::stopped, thread, &QThread::quit);

        QObject::connect(request, &Connectivity::SimpleCurlRequest::stopped, request, &Connectivity::SimpleCurlRequest::deleteLater);
        QObject::connect(thread, &QThread::finished, thread, &QThread::deleteLater);

        QObject::connect(request, &Connectivity::SimpleCurlRequest::requestFinished, this, &ShutterstockQueryEngine::requestFinishedHandler);

        thread->start();
    }

    void ShutterstockQueryEngine::requestFinishedHandler(bool success) {
        LOG_INFO << "success:" << success;

        Connectivity::SimpleCurlRequest *request = qobject_cast<Connectivity::SimpleCurlRequest *>(sender());

        if (success) {
            QJsonParseError error;

            QJsonDocument document = QJsonDocument::fromJson(request->getResponseData(), &error);

            if (error.error == QJsonParseError::NoError) {
                QJsonObject jsonObject = document.object();
                QJsonValue dataValue = jsonObject["data"];

                if (dataValue.isArray()) {
                    std::vector<std::shared_ptr<SuggestionArtwork> > suggestionArtworks;
                    parseResponse(dataValue.toArray(), suggestionArtworks);
                    setResults(suggestionArtworks);
                    emit resultsAvailable();
                }
            } else {
                LOG_WARNING << "parsing error:" << error.errorString();
                emit errorReceived(tr("Can't parse the response"));
            }
        } else {
            LOG_WARNING << "error:" << request->getErrorString();
            emit errorReceived(request->getErrorString());
        }

        request->dispose();
    }

    void ShutterstockQueryEngine::parseResponse(const QJsonArray &jsonArray,
                                                std::vector<std::shared_ptr<SuggestionArtwork> > &suggestionArtworks) {
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

    QUrl ShutterstockQueryEngine::buildQuery(const SearchQuery &query) const {
        QUrlQuery urlQuery;

        urlQuery.addQueryItem("language", "en");
        urlQuery.addQueryItem("view", "full");
        urlQuery.addQueryItem("per_page", QString::number(query.m_MaxResults));
        urlQuery.addQueryItem("sort", "popular");
        urlQuery.addQueryItem("query", query.m_SearchTerms.join(' '));

        if (!Common::HasFlag(query.m_Flags, Suggestion::AllImages)) {
            urlQuery.addQueryItem("image_type", resultsTypeToString(query.m_Flags));
        }

        QUrl url;
        url.setUrl(QLatin1String("https://api.shutterstock.com/v2/images/search"));
        url.setQuery(urlQuery);
        return url;
    }

    QString ShutterstockQueryEngine::resultsTypeToString(Common::flag_t queryFlags) const {
        if (Common::HasFlag(queryFlags, Suggestion::Photos)) { return QLatin1String("photo"); }
        else if (Common::HasFlag(queryFlags, Suggestion::Vectors)) { return QLatin1String("vector"); }
        else if (Common::HasFlag(queryFlags, Suggestion::Illustrations)) { return QLatin1String("illustration"); }
        else { return QString(); }
    }
}


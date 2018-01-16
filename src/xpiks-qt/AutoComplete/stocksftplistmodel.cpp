/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "stocksftplistmodel.h"
#include <QStandardPaths>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include "../Models/abstractconfigupdatermodel.h"
#include "../Common/defines.h"
#include "../Helpers/localconfig.h"
#include "../Connectivity/apimanager.h"

#define OVERWRITE_STOCKS_CONFIG false
#define LOCAL_STOCKS_LIST_FILE QLatin1String("stocks_ftp.json")

#define OVERWRITE_KEY QLatin1String("overwrite")
#define FTP_ARRAY_KEY QLatin1String("stocks_ftp")
#define FTP_NAME_KEY QLatin1String("name")
#define FTP_ADDRESS_KEY QLatin1String("ftp")

namespace AutoComplete {
    StocksFtpListModel::StocksFtpListModel():
        Models::AbstractConfigUpdaterModel(OVERWRITE_STOCKS_CONFIG)
    { }

    void StocksFtpListModel::initializeConfigs() {
        LOG_DEBUG << "#";

        QString localConfigPath;

        QString appDataPath = XPIKS_USERDATA_PATH;
        if (!appDataPath.isEmpty()) {
            QDir appDataDir(appDataPath);
            localConfigPath = appDataDir.filePath(LOCAL_STOCKS_LIST_FILE);
        } else {
            localConfigPath = LOCAL_STOCKS_LIST_FILE;
        }

        auto &apiManager = Connectivity::ApiManager::getInstance();
        QString remoteAddress = apiManager.getStocksACSourceAddr();
        AbstractConfigUpdaterModel::initializeConfigs(remoteAddress, localConfigPath);
    }

    void StocksFtpListModel::processRemoteConfig(const QJsonDocument &remoteDocument, bool overwriteLocal) {
        LOG_DEBUG << "#";
        bool overwrite = false;

        if (!overwriteLocal && remoteDocument.isObject()) {
            QJsonObject rootObject = remoteDocument.object();
            if (rootObject.contains(OVERWRITE_KEY)) {
                QJsonValue overwriteValue = rootObject[OVERWRITE_KEY];
                if (overwriteValue.isBool()) {
                    overwrite = overwriteValue.toBool();
                    LOG_DEBUG << "Overwrite flag present in the config:" << overwrite;
                } else {
                    LOG_WARNING << "Overwrite flag is not boolean";
                }
            }
        } else {
            overwrite = overwriteLocal;
        }

        Models::AbstractConfigUpdaterModel::processRemoteConfig(remoteDocument, overwrite);
    }

    bool StocksFtpListModel::processLocalConfig(const QJsonDocument &document) {
        Q_ASSERT(m_StocksHash.empty());
        bool result = parseConfig(document);
        return result;
    }

    void StocksFtpListModel::processMergedConfig(const QJsonDocument &document) {
#ifdef INTEGRATION_TESTS
        parseConfig(document);
#else
        Q_UNUSED(document);
#endif
    }

    bool StocksFtpListModel::parseConfig(const QJsonDocument &document) {
        bool anyError = false;

        do {
            if (!document.isObject()) {
                LOG_WARNING << "Json document is not an object";
                anyError = true;
                break;
            }

            QJsonObject rootObject = document.object();
            if (!rootObject.contains(FTP_ARRAY_KEY)) {
                LOG_WARNING << "There's no ftp array key in json";
                anyError = true;
                break;
            }

            QJsonValue ftpArrayValue = rootObject[FTP_ARRAY_KEY];
            if (!ftpArrayValue.isArray()) {
                LOG_WARNING << "Ftp array object is not an array";
                anyError = true;
                break;
            }

            QJsonArray ftpArray = ftpArrayValue.toArray();
            parseFtpArray(ftpArray);

        } while (false);

        return anyError;
    }

    void StocksFtpListModel::parseFtpArray(const QJsonArray &array) {
        LOG_DEBUG << array.size() << "ftp hosts";
        QHash<QString, QString> hash;
        QStringList keys;
        int size = array.size();
        keys.reserve(size);

        for (int i = 0; i < size; ++i) {
            QJsonValue item = array.at(i);

            if (!item.isObject()) { continue; }

            QJsonObject ftpItem = item.toObject();
            if (!ftpItem.contains(FTP_ADDRESS_KEY) ||
                    !ftpItem.contains(FTP_NAME_KEY)) {
                continue;
            }

            QJsonValue addressValue = ftpItem[FTP_ADDRESS_KEY];
            QJsonValue nameValue = ftpItem[FTP_NAME_KEY];

            if (!addressValue.isString() ||
                    !nameValue.isString()) {
                continue;
            }

            keys.append(nameValue.toString());
            hash[nameValue.toString()] = addressValue.toString();
        }

        if (!hash.isEmpty()) {
            LOG_INFO << "Replacing stocks hash with" << hash.size() << "key-values";
            m_StocksHash.swap(hash);
            m_StockNames.swap(keys);
            emit stocksListUpdated();
        } else {
            LOG_INTEGR_TESTS_OR_DEBUG << "Hash is empty!";
        }
    }

    int StocksFtpListModel::operator ()(const QJsonObject &val1, const QJsonObject &val2) {
        bool areEqual = false;

        if (val1.contains(FTP_NAME_KEY) &&
                val2.contains(FTP_NAME_KEY)) {
            QJsonValue nameValue1 = val1[FTP_NAME_KEY];
            QJsonValue nameValue2 = val2[FTP_NAME_KEY];

            if (nameValue1.isString() && nameValue2.isString()) {
                areEqual = nameValue1.toString() == nameValue2.toString();
            }
        }

        return (areEqual ? 0 : -1);
    }
}

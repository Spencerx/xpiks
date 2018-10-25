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

#include <algorithm>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonValueRef>
#include <QLatin1String>
#include <Qt>
#include <QtDebug>
#include <QtGlobal>

#include "Common/isystemenvironment.h"
#include "Common/logging.h"
#include "Connectivity/apimanager.h"
#include "Microstocks/stockftpoptions.h"
#include "Models/Connectivity/abstractconfigupdatermodel.h"

#define OVERWRITE_STOCKS_CONFIG false
#define LOCAL_STOCKS_LIST_FILE QLatin1String("stocks_ftp.json")

#define OVERWRITE_KEY QLatin1String("overwrite")
#define FTP_ARRAY_KEY QLatin1String("stocks_ftp")
#define FTP_NAME_KEY QLatin1String("name")
#define FTP_ADDRESS_KEY QLatin1String("ftp")
#define FTP_IMAGES_DIR_KEY QLatin1String("images_dir")
#define FTP_VECTORS_DIR_KEY QLatin1String("vectors_dir")
#define FTP_VIDEOS_DIR_KEY QLatin1String("videos_dir")
#define FTP_ZIP_VECTORS_KEY QLatin1String("zip_vector")

namespace Microstocks {
    StocksFtpListModel::StocksFtpListModel(Common::ISystemEnvironment &environment):
        Models::AbstractConfigUpdaterModel(
            environment.path({LOCAL_STOCKS_LIST_FILE}),
            Connectivity::ApiManager::getInstance().getStocksACSourceAddr(),
            OVERWRITE_STOCKS_CONFIG,
            environment.getIsInMemoryOnly())
    { }

    std::shared_ptr<StockFtpOptions> StocksFtpListModel::findFtpOptions(const QString &title) const {
        auto it = std::find_if(m_StocksList.begin(), m_StocksList.end(),
                               [&](std::shared_ptr<StockFtpOptions> const &current) {
                return QString::compare(title, current->m_Title, Qt::CaseInsensitive) == 0; });

        std::shared_ptr<StockFtpOptions> result;
        if (it != m_StocksList.end()) {
            result = *it;
        }

        return result;
    }

    QStringList StocksFtpListModel::getStockNamesList() const {
        QStringList names;
        names.reserve((int)m_StocksList.size());
        for (auto &stock: m_StocksList) {
            names.append(stock->m_Title);
        }
        return names;
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
        Q_ASSERT(m_StocksList.empty());
        bool result = parseConfig(document);
        return result;
    }

    void StocksFtpListModel::processMergedConfig(const QJsonDocument &document) {
        parseConfig(document);
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
        std::vector<std::shared_ptr<StockFtpOptions> > ftpOptionsList;
        const int size = array.size();
        ftpOptionsList.reserve(size);

        for (int i = 0; i < size; ++i) {
            QJsonValue item = array.at(i);
            if (!item.isObject()) { continue; }

            auto ftpOptions = std::make_shared<StockFtpOptions>();
            QJsonObject ftpItem = item.toObject();

            QJsonValue addressValue = ftpItem[FTP_ADDRESS_KEY];
            if (!addressValue.isString()) { continue; }
            ftpOptions->m_FtpAddress = addressValue.toString();

            QJsonValue nameValue = ftpItem[FTP_NAME_KEY];
            if (!nameValue.isString()) { continue; }
            ftpOptions->m_Title = nameValue.toString();

            QJsonValue imagesDirValue = ftpItem[FTP_IMAGES_DIR_KEY];
            if (imagesDirValue.isString()) {
                ftpOptions->m_ImagesDir = imagesDirValue.toString();
            }

            QJsonValue vectorsDirValue = ftpItem[FTP_VECTORS_DIR_KEY];
            if (vectorsDirValue.isString()) {
                ftpOptions->m_VectorsDir = vectorsDirValue.toString();
            }

            QJsonValue videosDirValue = ftpItem[FTP_VIDEOS_DIR_KEY];
            if (videosDirValue.isString()) {
                ftpOptions->m_VideosDir = videosDirValue.toString();
            }

            QJsonValue zipVectorValue = ftpItem[FTP_ZIP_VECTORS_KEY];
            ftpOptions->m_ZipVector = zipVectorValue.isBool() && zipVectorValue.toBool();

            ftpOptionsList.emplace_back(ftpOptions);
        }

        if (!ftpOptionsList.empty()) {
            LOG_INFO << "Replacing stocks list with" << ftpOptionsList.size() << "items";
            m_StocksList.swap(ftpOptionsList);
            emit stocksListUpdated();
        } else {
            LOG_VERBOSE_OR_DEBUG << "List is empty!";
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

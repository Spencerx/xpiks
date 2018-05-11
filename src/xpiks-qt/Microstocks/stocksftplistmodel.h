/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef STOCKSFTPLISTMODEL_H
#define STOCKSFTPLISTMODEL_H

#include <QHash>
#include <QList>
#include <QJsonDocument>
#include <QJsonArray>
#include <vector>
#include "../Models/abstractconfigupdatermodel.h"
#include "../Common/isystemenvironment.h"

namespace Microstocks {
    struct StockFtpOptions {
        QString m_Title;
        QString m_FtpAddress;
        QString m_ImagesDir;
        QString m_VectorsDir;
        QString m_VideosDir;
        bool m_ZipVector = false;
    };

    class StocksFtpListModel: public Models::AbstractConfigUpdaterModel
    {
        Q_OBJECT
    public:
        StocksFtpListModel(Common::ISystemEnvironment &environment);

    public:
        QString getFtpAddress(const QString &stockName) const;
        const QStringList &getStockNamesList() const { return m_StockNames; }

        // AbstractConfigUpdaterModel interface
    protected:
        virtual void processRemoteConfig(const QJsonDocument &remoteDocument, bool overwriteLocal) override;
        virtual bool processLocalConfig(const QJsonDocument &document) override;
        virtual void processMergedConfig(const QJsonDocument &document) override;

    private:
        bool parseConfig(const QJsonDocument &document);
        void parseFtpArray(const QJsonArray &array);

    signals:
        void stocksListUpdated();

        // CompareValuesJson interface
    public:
        virtual int operator ()(const QJsonObject &val1, const QJsonObject &val2) override;

    private:
        std::vector<StockFtpOptions> m_StocksList;
        QStringList m_StockNames;
    };
}

#endif // STOCKSFTPLISTMODEL_H

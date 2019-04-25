/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef STOCKSFTPLISTMODEL_H
#define STOCKSFTPLISTMODEL_H

#include <memory>
#include <vector>

#include <QJsonArray>
#include <QJsonDocument>
#include <QObject>
#include <QString>
#include <QStringList>

#include "Models/Connectivity/abstractconfigupdatermodel.h"

class QJsonObject;

namespace Common {
    class ISystemEnvironment;
}

namespace Microstocks {
    struct StockFtpOptions;

    class StocksFtpListModel: public Models::AbstractConfigUpdaterModel
    {
        Q_OBJECT
    public:
        StocksFtpListModel(Common::ISystemEnvironment &environment);

    public:
        std::shared_ptr<StockFtpOptions> findFtpOptionsByTitle(const QString &title) const;
        std::shared_ptr<StockFtpOptions> findFtpOptionsByID(int id) const;
        QStringList getStockNamesList() const;

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
        std::vector<std::shared_ptr<StockFtpOptions> > m_StocksList;
    };
}

#endif // STOCKSFTPLISTMODEL_H

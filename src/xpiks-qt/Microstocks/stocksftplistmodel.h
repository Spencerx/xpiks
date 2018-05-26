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
#include <memory>
#include "../Models/abstractconfigupdatermodel.h"
#include "../Common/isystemenvironment.h"
#include "stockftpoptions.h"

namespace Microstocks {
    class StocksFtpListModel: public Models::AbstractConfigUpdaterModel
    {
        Q_OBJECT
    public:
        StocksFtpListModel(Common::ISystemEnvironment &environment);

    public:
        std::shared_ptr<StockFtpOptions> findFtpOptions(const QString &title) const;
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

/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef PLUGINSDATABASEMANAGER_H
#define PLUGINSDATABASEMANAGER_H

#include "../Helpers/idatabasemanager.h"
#include "../Helpers/database.h"
#include "../Common/isystemenvironment.h"

namespace Plugins {
    class PluginDatabaseManager: public Helpers::IDatabaseManager
    {
    public:
        PluginDatabaseManager(Common::ISystemEnvironment &environment, Helpers::DatabaseManager *dbManager);

        // IDatabaseManager interface
    public:
        virtual std::shared_ptr<Helpers::Database> openDatabase(const QString &dbName) override;

    private:
        Common::ISystemEnvironment &m_Environment;
        Helpers::DatabaseManager *m_DatabaseManager;
    };
}

#endif // PLUGINSDATABASEMANAGER_H

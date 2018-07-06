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

#include "../Storage/idatabasemanager.h"
#include "../Storage/databasemanager.h"
#include "../Common/isystemenvironment.h"

namespace Plugins {
    class PluginDatabaseManager: public Storage::IDatabaseManager
    {
    public:
        PluginDatabaseManager(Common::ISystemEnvironment &environment, Storage::DatabaseManager &dbManager);

        // IDatabaseManager interface
    public:
        virtual std::shared_ptr<Storage::IDatabase> openDatabase(const QString &dbName) override;

    private:
        Common::ISystemEnvironment &m_Environment;
        Storage::DatabaseManager &m_DatabaseManager;
    };
}

#endif // PLUGINSDATABASEMANAGER_H

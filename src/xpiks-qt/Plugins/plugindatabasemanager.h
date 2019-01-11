/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef PLUGINSDATABASEMANAGER_H
#define PLUGINSDATABASEMANAGER_H

#include <memory>

#include <QString>

#include "Storage/idatabasemanager.h"

namespace Common {
    class ISystemEnvironment;
}

namespace Storage {
    class DatabaseManager;
    class IDatabase;
}

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

/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "plugindatabasemanager.h"

namespace Plugins {
    PluginDatabaseManager::PluginDatabaseManager(Common::ISystemEnvironment &environment,
                                                 Storage::DatabaseManager &dbManager):
        m_Environment(environment),
        m_DatabaseManager(dbManager)
    { }

    std::shared_ptr<Storage::IDatabase> PluginDatabaseManager::openDatabase(const QString &dbName) {
        return m_DatabaseManager.openDatabase(m_Environment, dbName);
    }
}

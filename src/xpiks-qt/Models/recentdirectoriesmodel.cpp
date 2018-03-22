/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "recentdirectoriesmodel.h"
#include "../Helpers/constants.h"

#define MAX_RECENT_DIRECTORIES 5

namespace Models {
    RecentDirectoriesModel::RecentDirectoriesModel():
        RecentItemsModel(MAX_RECENT_DIRECTORIES),
        m_State("recentdirs")
    {
    }

    void RecentDirectoriesModel::initialize(Common::ISystemEnvironment &environment) {
        m_State.init(environment);

        QString recentDirectories = m_State.getString(Constants::recentDirectories);
        deserializeItems(recentDirectories);
    }

    void RecentDirectoriesModel::sync() {
        QString recentDirectories = serializeItems();
        m_State.setValue(Constants::recentDirectories, recentDirectories);
        m_State.sync();
    }
}


/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "recentfilesmodel.h"
#include "../Helpers/constants.h"

#define MAX_RECENT_FILES 10

namespace Models {
    RecentFilesModel::RecentFilesModel(Common::ISystemEnvironment &environment):
        RecentItemsModel(MAX_RECENT_FILES),
        m_State("recentfiles", environment)
    {
    }

    void RecentFilesModel::initialize() {
        m_State.init();

        QString recentFiles = m_State.getString(Constants::recentFiles);
        deserializeItems(recentFiles);
    }

    void RecentFilesModel::add(const MetadataIO::ArtworksSnapshot &snapshot) {
        int maxFiles = getMaxRecentItems();
        const int length = (int)snapshot.size();
        int first = qMax(0, length - maxFiles);

        for (; first < length; ++first) {
            auto *artwork = snapshot.get(first);
            pushItem(artwork->getFilepath());
        }
    }

    void RecentFilesModel::sync() {
        QString recentFiles = serializeItems();
        m_State.setValue(Constants::recentFiles, recentFiles);
        m_State.sync();
    }
}

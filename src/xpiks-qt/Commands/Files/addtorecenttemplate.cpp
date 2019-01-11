/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "addtorecenttemplate.h"

#include <QtDebug>

#include "Common/logging.h"
#include "Models/Session/recentfilesmodel.h"

namespace Commands {
    AddToRecentTemplate::AddToRecentTemplate(Models::RecentFilesModel &recentFilesModel):
        m_RecentFilesModel(recentFilesModel)
    {
    }

    void AddToRecentTemplate::execute(const Artworks::ArtworksSnapshot &snapshot) {
        LOG_DEBUG << "#";
        m_RecentFilesModel.add(snapshot);
    }
}

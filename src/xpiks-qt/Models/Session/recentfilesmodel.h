/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef RECENTFILESMODEL_H
#define RECENTFILESMODEL_H

#include "recentitemsmodel.h"
#include <Common/statefulentity.h>

namespace Common {
    class ISystemEnvironment;
}

namespace Artworks {
    class ArtworksSnapshot;
}

namespace Models {
    class RecentFilesModel : public RecentItemsModel
    {
        Q_OBJECT
    public:
        RecentFilesModel(Common::ISystemEnvironment &environment);

    public:
        virtual void initialize() override;

    public:
        void add(const Artworks::ArtworksSnapshot &snapshot);

    protected:
        virtual void sync() override;

    private:
        Common::StatefulEntity m_State;
    };
}

#endif // RECENTFILESMODEL_H

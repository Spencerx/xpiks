/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef RECENTDIRECTORIESMODEL_H
#define RECENTDIRECTORIESMODEL_H

#include <QObject>
#include <QString>

#include "Common/statefulentity.h"
#include "Models/Session/recentitemsmodel.h"

namespace Common {
    class ISystemEnvironment;
}

namespace Models {
    class RecentDirectoriesModel : public RecentItemsModel
    {
        Q_OBJECT
    public:
        RecentDirectoriesModel(Common::ISystemEnvironment &environment);

    public:
        virtual void initialize() override;

    public:
        Q_INVOKABLE QUrl getLatestItem() const;

    protected:
        virtual void sync() override;

    private:
        Common::StatefulEntity m_State;
    };
}

#endif // RECENTDIRECTORIESMODEL_H

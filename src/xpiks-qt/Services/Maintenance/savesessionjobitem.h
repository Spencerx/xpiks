/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SAVESESSIONJOBITEM_H
#define SAVESESSIONJOBITEM_H

#include "imaintenanceitem.h"

namespace Models {
    class SessionManager;
}

namespace Artworks {
    class SessionSnapshot;
}

namespace Maintenance {
    class SaveSessionJobItem : public IMaintenanceItem
    {
    public:
        SaveSessionJobItem(std::unique_ptr<Artworks::SessionSnapshot> &sessionSnapshot,
                           Models::SessionManager &sessionManager);

    public:
        virtual void processJob() override;

    private:
        void doSaveSession();

    private:
        std::unique_ptr<Artworks::SessionSnapshot> m_SessionSnapshot;
        Models::SessionManager &m_SessionManager;
    };
}

#endif // SAVESESSIONJOBITEM_H

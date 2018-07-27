/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <Models/Session/sessionmanager.h>
#include <Artworks/artworkssnapshot.h>
#include "savesessionjobitem.h"

namespace Maintenance {
    SaveSessionJobItem::SaveSessionJobItem(std::unique_ptr<Artworks::SessionSnapshot> &sessionSnapshot,
                                           Models::SessionManager &sessionManager):
        m_SessionSnapshot(std::move(sessionSnapshot)),
        m_SessionManager(sessionManager)
    {
    }

    void SaveSessionJobItem::processJob() {
        LOG_DEBUG << "#";
        doSaveSession();
    }

    void SaveSessionJobItem::doSaveSession() {
        m_SessionManager.save(m_SessionSnapshot);
    }
}

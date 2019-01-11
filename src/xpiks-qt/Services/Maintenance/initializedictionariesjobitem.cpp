/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "initializedictionariesjobitem.h"

#include <QtDebug>
#include <QtGlobal>

#include "Common/logging.h"
#include "Helpers/asynccoordinator.h"
#include "Services/Translation/translationmanager.h"

namespace Maintenance {
    InitializeDictionariesJobItem::InitializeDictionariesJobItem(Translation::TranslationManager &translationManager,
                                                                 Helpers::AsyncCoordinator &initCoordinator):
        m_TranslationManager(translationManager),
        m_InitCoordinator(initCoordinator)
    {
    }

    void InitializeDictionariesJobItem::processJob() {
        LOG_DEBUG << "#";

        Helpers::AsyncCoordinatorUnlocker unlocker(m_InitCoordinator);
        Q_UNUSED(unlocker);

        doInitializeDictionaries();
    }

    void InitializeDictionariesJobItem::doInitializeDictionaries() {
        m_TranslationManager.doInitializeDictionaries();
        m_TranslationManager.initializationFinished();
    }
}

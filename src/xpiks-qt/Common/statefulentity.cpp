/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "statefulentity.h"

#include <Qt>
#include <QtDebug>
#include <QtGlobal>

#include "Common/isystemenvironment.h"
#include "Common/logging.h"
#include "Helpers/constants.h"
#include "Helpers/jsonobjectmap.h"
#include "Helpers/localconfig.h"

namespace Common {
    StatefulEntity::StatefulEntity(const QString &stateName, ISystemEnvironment &environment):
        m_StateName(stateName),
        m_Config(environment.path({Constants::STATES_DIR, QString("%1.json").arg(stateName)}),
                 environment.getIsInMemoryOnly()),
        m_StateMap(std::make_shared<Helpers::JsonObjectMap>()),
        m_InitCounter(0)
    {
        Q_ASSERT(!stateName.endsWith(".json", Qt::CaseInsensitive));
    }

    StatefulEntity::~StatefulEntity() {
        Q_ASSERT(m_InitCounter.load() > 0);
    }

    void StatefulEntity::init() {
        LOG_DEBUG << m_StateName;

        if (m_InitCounter.fetch_add(1) == 0) {
            m_StateMap = m_Config.readMap();
        } else {
            LOG_WARNING << "Attempt to initialize state" << m_StateName << "twice";
            Q_ASSERT(false);
        }
    }

    void StatefulEntity::sync() {
        LOG_DEBUG << m_StateName;

        if (m_InitCounter.load() > 0) {
            m_Config.writeMap(m_StateMap);
        } else {
            LOG_WARNING << "State" << m_StateName << "is not initialized!";
            Q_ASSERT(false);
        }
    }
}

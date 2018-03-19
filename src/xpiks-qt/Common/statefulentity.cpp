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
#include <QDir>
#include "../Helpers/constants.h"
#include "../Helpers/filehelpers.h"

namespace Common {
    StatefulEntity::StatefulEntity(const QString &stateName):
        m_StateName(stateName),
        m_StateMap(new Helpers::JsonObjectMap())
    {
        Q_ASSERT(!stateName.endsWith(".json", Qt::CaseInsensitive));
    }

    void StatefulEntity::init(ISystemEnvironment &environment) {
        LOG_DEBUG << m_StateName;

        QString filename = QString("%1.json").arg(m_StateName);

        QString localConfigPath = environment.path({Constants::STATES_DIR, filename});
        m_StateConfig.initConfig(localConfigPath);

        QJsonDocument &doc = m_StateConfig.getConfig();
        if (doc.isObject()) {
            QJsonObject json = doc.object();
            m_StateMap.reset(new Helpers::JsonObjectMap(json));
        }

#ifdef QT_DEBUG
        m_Initialized = true;
#endif
    }

    void StatefulEntity::sync() {
        LOG_DEBUG << m_StateName;
#ifdef QT_DEBUG
        Q_ASSERT(m_Initialized);
#endif

        // do not use dropper
        // Helpers::LocalConfigDropper dropper(&m_StateConfig);

        QJsonDocument doc;
        QJsonObject json = m_StateMap->json();
        doc.setObject(json);

        m_StateConfig.setConfig(doc);
        m_StateConfig.saveToFile();
    }
}

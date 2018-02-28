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
    StatefulEntity::StatefulEntity(const QString &stateName, ISystemEnvironment &environment):
        m_Environment(environment),
        m_StateName(stateName)
    {
        Q_ASSERT(!stateName.endsWith(".json", Qt::CaseInsensitive));
    }

    void StatefulEntity::init() {
        LOG_DEBUG << m_StateName;

#if defined(QT_DEBUG)
        QString filename = QString("debug_%1.json").arg(m_StateName);
#elif defined(INTEGRATION_TESTS)
        QString filename = QString("tests_%1.json").arg(m_StateName);
#else
        QString filename = QString("%1.json").arg(m_StateName);
#endif

        QString localConfigPath = m_Environment.fileInDir(filename, Constants::STATES_DIR);
        m_StateConfig.initConfig(localConfigPath);

        QJsonDocument &doc = m_StateConfig.getConfig();
        if (doc.isObject()) {
            m_StateJson = doc.object();
        }
    }

    void StatefulEntity::sync() {
        LOG_DEBUG << m_StateName;

        // do not use dropper
        // Helpers::LocalConfigDropper dropper(&m_StateConfig);

        QJsonDocument doc;
        doc.setObject(m_StateJson);

        m_StateConfig.setConfig(doc);
        m_StateConfig.saveToFile();
    }
}

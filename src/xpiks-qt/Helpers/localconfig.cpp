/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "localconfig.h"
#include <QDir>
#include <QStandardPaths>

namespace Helpers {
    LocalConfig::LocalConfig(const QString &filepath, bool memoryOnly):
        m_FilePath(filepath),
        m_MemoryOnly(memoryOnly)
    {
        Q_ASSERT(!m_FilePath.isEmpty());
    }

    QJsonDocument LocalConfig::readConfig() {
        QJsonDocument config;

        QFile file(m_FilePath);

        if (file.open(QIODevice::ReadOnly)) {
            QString text = QString::fromUtf8(file.readAll());
            file.close();
            config = QJsonDocument::fromJson(text.toUtf8());
        } else {
            LOG_WARNING << "Opening file" << m_FilePath << "failed";
        }

        return config;
    }

    std::shared_ptr<JsonObjectMap> LocalConfig::readMap() {
        std::shared_ptr<JsonObjectMap> stateMap;
        QJsonDocument config = readConfig();
        if (config.isObject()) {
            QJsonObject json = config.object();
            stateMap.reset(new Helpers::JsonObjectMap(json));
        } else {
            stateMap.reset(new Helpers::JsonObjectMap());
        }
        return stateMap;
    }

    bool LocalConfig::writeMap(const std::shared_ptr<JsonObjectMap> &map) {
        if (m_MemoryOnly) { return true; }
        if (m_FilePath.isEmpty()) { return false; }

        QJsonDocument doc;
        QJsonObject json = map->json();
        doc.setObject(json);
        bool success = writeConfig(doc);
        return success;
    }

    bool LocalConfig::writeConfig(const QJsonDocument &config) {
        if (m_MemoryOnly) { return true; }
        if (m_FilePath.isEmpty()) { return false; }

        bool success = false;
        QFile file(m_FilePath);

        if (file.open(QIODevice::WriteOnly)) {
            file.write(config.toJson(QJsonDocument::Indented));
            file.close();
            success = true;
        } else {
            LOG_WARNING << "Opening file" << m_FilePath << "failed";
        }

        return success;
    }
}

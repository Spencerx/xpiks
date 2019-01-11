/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef LOCALCONFIG_H
#define LOCALCONFIG_H

#include <memory>

#include <QJsonDocument>
#include <QString>

namespace Helpers {
    class JsonObjectMap;

    class LocalConfig {
    public:
        LocalConfig(const QString &filepath, bool memoryOnly=false);

    public:
        QJsonDocument readConfig();
        std::shared_ptr<JsonObjectMap> readMap();

    public:
        bool writeMap(const std::shared_ptr<JsonObjectMap> &map);
        bool writeConfig(const QJsonDocument &config);

    private:
        QString m_FilePath;
        bool m_MemoryOnly;
    };
}

#endif // LOCALCONFIG_H

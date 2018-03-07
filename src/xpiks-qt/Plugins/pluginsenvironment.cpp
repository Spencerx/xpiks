/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "pluginsenvironment.h"
#include <QDir>
#include "../Helpers/filehelpers.h"

PluginsEnvironment::PluginsEnvironment(ISystemEnvironment &systemEnvironment, const QString &pluginsDir):
    m_SystemEnvironment(systemEnvironment),
    m_PluginsDir(pluginsDir)
{
}

QString PluginsEnvironment::root() const {
    return m_SystemEnvironment.path({m_PluginsDir});
}

QString PluginsEnvironment::path(const QStringList &path) {
    QStringList extended;
    extended << m_PluginsDir;
    extended.append(path);
    return m_SystemEnvironment.path(extended);
}

bool PluginsEnvironment::ensureDirExists(const QString &name) {
    QString dirPath = m_SystemEnvironment.path({m_PluginsDir, name});
    bool result = Helpers::ensureDirectoryExists(dirPath);
    return result;
}

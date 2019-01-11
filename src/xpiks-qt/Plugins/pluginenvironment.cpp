/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "pluginenvironment.h"

#include <QFileInfo>
#include <QtDebug>
#include <QtGlobal>

#include "Common/logging.h"
#include "Helpers/constants.h"
#include "Helpers/filehelpers.h"

PluginEnvironment::PluginEnvironment(ISystemEnvironment &systemEnvironment,
                                     const QString &pluginsDir,
                                     const QString &fullPluginPath):
    m_SystemEnvironment(systemEnvironment),
    m_PluginsDir(pluginsDir),
    m_DataDirName("data")
{
    Q_ASSERT(!pluginsDir.isEmpty());
    Q_ASSERT(!fullPluginPath.isEmpty());

    QFileInfo fi(fullPluginPath);
    m_PluginDirName = fi.baseName();
}

void PluginEnvironment::initialize() {
    LOG_DEBUG << "Initializing in" << root();
    ensureDirExists(Constants::STATES_DIR);
    ensureDirExists(Constants::DB_DIR);
}

QString PluginEnvironment::root() const {
    return m_SystemEnvironment.path({m_PluginsDir, m_DataDirName, m_PluginDirName});
}

QString PluginEnvironment::path(const QStringList &path) {
    QStringList extended;
    extended << m_PluginsDir << m_DataDirName << m_PluginDirName;
    extended.append(path);
    return m_SystemEnvironment.path(extended);
}

bool PluginEnvironment::ensureDirExists(const QString &name) {
    QString dirPath = m_SystemEnvironment.path({m_PluginsDir, m_DataDirName, m_PluginDirName, name});
    bool result = Helpers::ensureDirectoryExists(dirPath);
    return result;
}

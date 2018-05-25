/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef PLUGINSENVIRONMENT_H
#define PLUGINSENVIRONMENT_H

#include "../Common/isystemenvironment.h"

class PluginEnvironment: public Common::ISystemEnvironment
{
public:
    PluginEnvironment(Common::ISystemEnvironment &systemEnvironment,
                      const QString &pluginsDir,
                      const QString &fullPluginPath);

public:
    void initialize();

    // ISystemEnvironment interface
public:
    virtual QString root() const override;
    virtual QString path(const QStringList &path) override;
    virtual bool ensureDirExists(const QString &name) override;
    virtual bool getIsInMemoryOnly() const override { return m_SystemEnvironment.getIsInMemoryOnly(); }
    virtual bool getIsRecoveryMode() const override { return m_SystemEnvironment.getIsRecoveryMode(); }

private:
    Common::ISystemEnvironment &m_SystemEnvironment;
    QString m_PluginsDir;
    QString m_PluginDirName;
    QString m_DataDirName;
};

#endif // PLUGINSENVIRONMENT_H

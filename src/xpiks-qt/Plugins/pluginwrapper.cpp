/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "pluginwrapper.h"
#include <QDir>
#include "xpiksplugininterface.h"
#include "ipluginaction.h"
#include "pluginactionsmodel.h"
#include "uiprovider.h"
#include "../Common/defines.h"
#include "../Helpers/constants.h"

namespace Plugins {
    PluginWrapper::PluginWrapper(const QString &filepath,
                                 XpiksPluginInterface *pluginInterface,
                                 int pluginID,
                                 Common::ISystemEnvironment &environment,
                                 UIProvider *realUIProvider,
                                 Helpers::DatabaseManager *databaseManager):
        m_PluginInterface(pluginInterface),
        m_PluginEnvironment(environment, Constants::PLUGINS_DIR, filepath),
        m_PluginDatabaseManager(m_PluginEnvironment, databaseManager),
        m_ActionsModel(pluginInterface->getExportedActions(), pluginID),
        m_NotificationFlags(pluginInterface->getDesiredNotificationFlags()),
        m_UIProviderSafe(pluginID, realUIProvider),
        m_PluginFilepath(filepath),
        m_PluginID(pluginID),
        m_PluginFlags(0),
        m_PrettyName(pluginInterface->getPrettyName()),
        m_VersionString(pluginInterface->getVersionString()),
        m_Author(pluginInterface->getAuthor())
    {
        Q_ASSERT(realUIProvider != nullptr);
        Q_ASSERT(databaseManager != nullptr);
    }

    PluginWrapper::~PluginWrapper() {
        LOG_DEBUG << m_PluginID;
    }

    bool PluginWrapper::initializePlugin() {
        LOG_INFO << getPrettyName() << getVersionString();

        bool result = false;
        m_PluginEnvironment.initialize();

        try {
            result = m_PluginInterface->initialize();
            setIsInitializedFlag(true);
        }
        catch(...) {
            LOG_WARNING << "Exception while initializing plugin";
        }

        return result;
    }

    void PluginWrapper::enablePlugin() {
        LOG_INFO << getPrettyName() << getVersionString();
        Q_ASSERT(getIsInitializedFlag());

        try {
            m_PluginInterface->enable();
            setIsEnabledFlag(true);
        }
        catch(...) {
            LOG_WARNING << "Exception while enabling plugin";
        }
    }

    void PluginWrapper::disablePlugin() {
        Q_ASSERT(getIsEnabledFlag());
        LOG_INFO << getPrettyName() << getVersionString();

        try {
            // set disabled in any case
            setIsEnabledFlag(false);
            m_PluginInterface->disable();
        }
        catch(...) {
            LOG_WARNING << "Exception while disabling plugin";
        }
    }

    void PluginWrapper::triggerActionSafe(int actionID) const {
        LOG_INFO << getPrettyName() << "executing action:" << actionID;

        try {
            if (getIsEnabled()) {
                m_PluginInterface->executeAction(actionID);
            } else {
                LOG_WARNING << getPrettyName() << "is disabled";
            }
        }
        catch (...) {
            LOG_WARNING << "Exception while triggering action for plugin ID" << m_PluginID;
        }
    }

    void PluginWrapper::finalizePlugin() {
        LOG_INFO << getPrettyName() << getVersionString();
        Q_ASSERT(getIsInitializedFlag());

        try {
            m_PluginInterface->finalize();
            setIsInitializedFlag(false);
        }
        catch (...) {
            LOG_WARNING << "Exception on finalization";
        }
    }

    void PluginWrapper::removePlugin() {
        LOG_INFO << getPrettyName() << getVersionString();
        setIsRemovedFlag(true);

        const QString pluginRootPath = m_PluginEnvironment.root();
        LOG_DEBUG << "About to remove:" << pluginRootPath;

        QDir dataRoot(pluginRootPath);
        bool result = dataRoot.removeRecursively();
        LOG_INFO << "Removed plugin's data:" << result;
    }

    void PluginWrapper::notifyPlugin(PluginNotificationFlags flag, const QVariant &data, void *pointer) {
        if (getIsEnabled()) {
            if (Common::HasFlag(m_NotificationFlags, flag)) {
                m_PluginInterface->onPropertyChanged(flag, data, pointer);
            } else {
                LOG_DEBUG << "Plugin" << m_PrettyName << "is not subscribed to" << (Common::flag_t)flag;
            }
        } else {
            LOG_DEBUG << "Plugin" << m_PrettyName << "is disabled";
        }
    }
}


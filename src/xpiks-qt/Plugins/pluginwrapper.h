/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef PLUGINWRAPPER_H
#define PLUGINWRAPPER_H

#include <QObject>
#include <QString>
#include <cstdint>
#include "pluginactionsmodel.h"
#include "sandboxeddependencies.h"
#include "iuiprovider.h"
#include "xpiksplugininterface.h"
#include "../Common/flags.h"
#include "pluginenvironment.h"
#include "../Helpers/database.h"
#include "plugindatabasemanager.h"
#include "../Common/isystemenvironment.h"
#include "../Common/flags.h"

namespace Plugins {
    class UIProvider;

    class PluginWrapper
    {
    public:
        PluginWrapper(const QString &filepath,
                      XpiksPluginInterface *pluginInterface,
                      int pluginID,
                      Common::ISystemEnvironment &environment,
                      UIProvider *realUIProvider,
                      Helpers::DatabaseManager *databaseManager);
        virtual ~PluginWrapper();

    private:
        enum PluginFlags {
            FlagIsInitialized = 1 << 0,
            FlagIsEnabled = 1 << 1,
            FlagIsRemoved = 1 << 2
        };

        inline bool getIsInitializedFlag() const { return Common::HasFlag(m_PluginFlags, FlagIsInitialized); }
        inline bool getIsEnabledFlag() const { return Common::HasFlag(m_PluginFlags, FlagIsEnabled); }
        inline bool getIsRemovedFlag() const { return Common::HasFlag(m_PluginFlags, FlagIsRemoved); }

        inline void setIsInitializedFlag(bool value) { Common::ApplyFlag(m_PluginFlags, value, FlagIsInitialized); }
        inline void setIsEnabledFlag(bool value) { Common::ApplyFlag(m_PluginFlags, value, FlagIsEnabled); }
        inline void setIsRemovedFlag(bool value) { Common::ApplyFlag(m_PluginFlags, value, FlagIsRemoved); }

    public:
        int getPluginID() const { return m_PluginID; }
        bool getIsEnabled() const { return getIsEnabledFlag() && !getIsRemovedFlag(); }
        bool getIsRemoved() const { return getIsRemovedFlag(); }
        const QString &getFilepath() const { return m_PluginFilepath; }
        const QString &getPrettyName() const { return m_PrettyName; }
        const QString &getVersionString() const { return m_VersionString; }
        const QString &getAuthor() const { return m_Author; }

        bool anyActionsProvided() const { return m_ActionsModel.size() > 0; }
        PluginActionsModel *getActionsModel() { return &m_ActionsModel; }
        IUIProvider *getUIProvider() { return &m_UIProviderSafe; }
        Helpers::IDatabaseManager *getDatabaseManager() { return &m_PluginDatabaseManager; }

    public:
        bool initializePlugin();
        void enablePlugin();
        void disablePlugin();

        void triggerActionSafe(int actionID) const;
        void finalizePlugin();
        void removePlugin();

        void notifyPlugin(PluginNotificationFlags flag, const QVariant &data, void *pointer);

    private:
        XpiksPluginInterface *m_PluginInterface;
        PluginEnvironment m_PluginEnvironment;
        PluginDatabaseManager m_PluginDatabaseManager;
        PluginActionsModel m_ActionsModel;
        Common::flag_t m_NotificationFlags;
        UIProviderSafe m_UIProviderSafe;
        QString m_PluginFilepath;
        int m_PluginID;
        Common::flag_t m_PluginFlags;
        const QString &m_PrettyName;
        const QString &m_VersionString;
        const QString &m_Author;
    };
}

#endif // PLUGINWRAPPER_H

/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef XPIKSPLUGININTERFACE_H
#define XPIKSPLUGININTERFACE_H

#include <memory>
#include <vector>

#include <QObject>
#include <QVariant>
#include <QtGlobal>

#include "Common/flags.h"

class QString;

namespace Commands {
    class ICommandManager;
}

namespace Common {
    class ISystemEnvironment;
}

namespace KeywordsPresets {
    class IPresetsManager;
}

namespace Microstocks {
    class IMicrostockServices;
}

namespace Models {
    class ICurrentEditableSource;
}

namespace Storage {
    class IDatabaseManager;
}

namespace Plugins {
    class IPluginAction;
    class IUIProvider;

    class XpiksPluginInterface {
    public:
        virtual ~XpiksPluginInterface() { }

    public:
        virtual const QString &getPrettyName() const = 0;
        virtual const QString &getVersionString() const = 0;
        virtual const QString &getAuthor() const = 0;

        // actions routines
    public:
        virtual std::vector<std::shared_ptr<IPluginAction> > getExportedActions() const { return std::vector<std::shared_ptr<IPluginAction> >(); }
        virtual bool executeAction(int actionID) { Q_UNUSED(actionID); return false; }

        // general routines
    public:
        virtual bool initialize(Common::ISystemEnvironment &environment) = 0;
        virtual void finalize() = 0;
        virtual void enable() = 0;
        virtual void disable() = 0;

        // notification handlers
    public:
        // properties of which plugin wants to be notified
        virtual Common::PluginNotificationFlags getDesiredNotificationFlags() const { return  Common::PluginNotificationFlags::None; }
        virtual void onPropertyChanged(Common::PluginNotificationFlags flag, const QVariant &data, void *pointer) { Q_UNUSED(flag); Q_UNUSED(data); Q_UNUSED(pointer); }

    public:
        virtual void injectCommandManager(Commands::ICommandManager *commandManager) { Q_UNUSED(commandManager); }
        virtual void injectUIProvider(IUIProvider *uiProvider) { Q_UNUSED(uiProvider); }
        virtual void injectPresetsManager(KeywordsPresets::IPresetsManager *presetsManager) { Q_UNUSED(presetsManager); }
        virtual void injectDatabaseManager(Storage::IDatabaseManager *databaseManager) { Q_UNUSED(databaseManager); }
        virtual void injectMicrostockServices(Microstocks::IMicrostockServices *microstockServices) { Q_UNUSED(microstockServices); }
        virtual void injectCurrentEditable(Models::ICurrentEditableSource *currentEditableSource) { Q_UNUSED(currentEditableSource); }
    };
}

#define XpiksPluginInterface_iid "Xpiks.Plugins.XpiksPluginInterface.v0.0.1"
Q_DECLARE_INTERFACE(Plugins::XpiksPluginInterface, XpiksPluginInterface_iid)

#endif // XPIKSPLUGININTERFACE_H

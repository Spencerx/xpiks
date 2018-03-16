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

#include <QString>
#include <QtPlugin>
#include <QVector>
#include <cstdint>
#include <vector>
#include <memory>
#include "../Commands/icommandmanager.h"
#include "../UndoRedo/iundoredomanager.h"
#include "ipluginaction.h"
#include "iuiprovider.h"
#include "../Common/isystemenvironment.h"
#include "../Common/iartworkssource.h"
#include "../KeywordsPresets/ipresetsmanager.h"
#include "../Common/flags.h"
#include "../Storage/idatabasemanager.h"

namespace Plugins {
    enum struct PluginNotificationFlags: Common::flag_t {
        None = 0,
        CurrentEditableChanged = 1 << 0,
        ActionUndone = 1 << 1,
        PresetsUpdated = 1 << 2
    };

    class XpiksPluginInterface {
    public:
        virtual ~XpiksPluginInterface() {}

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
        virtual Common::flag_t getDesiredNotificationFlags() const { return (Common::flag_t)PluginNotificationFlags::None; }
        virtual void onPropertyChanged(PluginNotificationFlags flag, const QVariant &data, void *pointer) { Q_UNUSED(flag); Q_UNUSED(data); Q_UNUSED(pointer); }

    public:
        virtual void injectCommandManager(Commands::ICommandManager *commandManager) { Q_UNUSED(commandManager); }
        virtual void injectUndoRedoManager(UndoRedo::IUndoRedoManager *undoRedoManager) { Q_UNUSED(undoRedoManager); }
        virtual void injectArtworksSource(Common::IArtworksSource *artworksSource) { Q_UNUSED(artworksSource); }
        virtual void injectUIProvider(IUIProvider *uiProvider) { Q_UNUSED(uiProvider); }
        virtual void injectPresetsManager(KeywordsPresets::IPresetsManager *presetsManager) { Q_UNUSED(presetsManager); }
        virtual void injectDatabaseManager(Storage::IDatabaseManager *databaseManager) { Q_UNUSED(databaseManager); }
    };
}

#define XpiksPluginInterface_iid "Xpiks.Plugins.XpiksPluginInterface.v0.0.1"
Q_DECLARE_INTERFACE(Plugins::XpiksPluginInterface, XpiksPluginInterface_iid)

#endif // XPIKSPLUGININTERFACE_H

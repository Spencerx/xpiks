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
#include "../Common/iartworkssource.h"
#include "../KeywordsPresets/ipresetsmanager.h"
#include "../Common/flags.h"

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
        virtual const std::vector<std::shared_ptr<IPluginAction> > &getExportedActions() const = 0;
        virtual bool executeAction(int actionID) = 0;

        // general routines
    public:
        virtual void initializePlugin() = 0;
        virtual void finalizePlugin() = 0;
        virtual void enablePlugin() = 0;
        virtual void disablePlugin() = 0;

        // notification handlers
    public:
        // properties of which plugin wants to be notified
        virtual Common::flag_t getDesiredNotificationFlags() const = 0;
        virtual void onPropertyChanged(PluginNotificationFlags flag, const QVariant &data, void *pointer) = 0;

    public:
        virtual void injectCommandManager(Commands::ICommandManager *commandManager) = 0;
        virtual void injectUndoRedoManager(UndoRedo::IUndoRedoManager *undoRedoManager) = 0;
        virtual void injectArtworksSource(Common::IArtworksSource *artworksSource) = 0;
        virtual void injectUIProvider(IUIProvider *uiProvider) = 0;
        virtual void injectPresetsManager(KeywordsPresets::IPresetsManager *presetsManager) = 0;
    };
}

#define XpiksPluginInterface_iid "Xpiks.Plugins.XpiksPluginInterface.v0.0.1"
Q_DECLARE_INTERFACE(Plugins::XpiksPluginInterface, XpiksPluginInterface_iid)

#endif // XPIKSPLUGININTERFACE_H

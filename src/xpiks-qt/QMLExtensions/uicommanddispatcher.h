/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef UICOMMANDDISPATCHER_H
#define UICOMMANDDISPATCHER_H

#include <unordered_map>
#include <QObject>
#include "iuicommanddispatcher.h"

namespace Commands {
    class ICommandManager;
}

class QJSValue;

namespace QMLExtensions {
    class UICommandDispatcher:
            public QObject,
            public IUICommandDispatcher
    {
        Q_OBJECT
    public:
        UICommandDispatcher(Commands::ICommandManager &commandManager, QObject *parent=0);

    public:
        Q_INVOKABLE void dispatch(int commandID, QJSValue const &value);
        Q_INVOKABLE QObject *getCommandTarget(int commandID);

    public:
        void registerCommands(std::initializer_list<std::shared_ptr<Commands::IUICommandTemplate>> commands);

        // IUICommandDispatcher interface
    public:
        virtual void registerCommand(std::shared_ptr<Commands::IUICommandTemplate> const &command) override;
        virtual void dispatchCommand(int commandID, QVariant const &value) override;

    private:
        std::unordered_map<int, std::shared_ptr<Commands::IUICommandTemplate>> m_CommandsMap;
        Commands::ICommandManager &m_CommandManager;
    };
}

#endif // UICOMMANDDISPATCHER_H

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
#include <QMutex>
#include <memory>
#include "uiaction.h"
#include <Commands/Base/iuicommandtemplate.h>

namespace Commands {
    class ICommandManager;
}

class QJSValue;

namespace QMLExtensions {
    class UICommandDispatcher: public QObject
    {
        Q_OBJECT
    public:
        UICommandDispatcher(Commands::ICommandManager &commandManager, QObject *parent=0);

    public:
        Q_INVOKABLE void dispatch(int commandID, QJSValue const &value);
        Q_INVOKABLE QObject *getCommandTarget(int commandID);

    public:
        void registerCommands(std::initializer_list<std::shared_ptr<Commands::IUICommandTemplate>> commands);

    public:
        void registerCommand(std::shared_ptr<Commands::IUICommandTemplate> const &command);
#if defined(UI_TESTS) || defined(INTEGRATION_TESTS)
        void dispatchCommand(int commandID);
#endif

    signals:
        void actionsAvailable();
        void dispatched(int commandID, QJSValue const &value);

    private slots:
        void onActionsAvailable();

    private:
        void processAction(UIAction const &action);

    private:
        std::unordered_map<int, std::shared_ptr<Commands::IUICommandTemplate>> m_CommandsMap;
        QMutex m_Mutex;
        std::vector<UIAction> m_ActionsQueue;
        Commands::ICommandManager &m_CommandManager;
    };
}

#endif // UICOMMANDDISPATCHER_H

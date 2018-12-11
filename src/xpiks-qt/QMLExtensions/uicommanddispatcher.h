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

#include <initializer_list>
#include <memory>
#include <unordered_map>
#include <vector>

#include <QMutex>
#include <QObject>
#include <QString>
#include <QVariant>

namespace Commands {
    class ICommandManager;
    class IUICommandTemplate;
}

class QJSValue;

namespace QMLExtensions {
    class IUICommandMiddlware;
    struct UIAction;

    class UICommandDispatcher: public QObject
    {
        Q_OBJECT
    public:
        UICommandDispatcher(Commands::ICommandManager &commandManager, QObject *parent=nullptr);

    public:
        Q_INVOKABLE void dispatch(int commandID, QJSValue const &value);
        Q_INVOKABLE QObject *getCommandTarget(int commandID);

#if defined(INTEGRATION_TESTS) || defined(UI_TESTS)
    public:
        void dispatchCommand(int commandID, const QVariant &value);
        void processAll() { onActionsAvailable(); }
#endif

    public:
        void registerCommands(std::initializer_list<std::shared_ptr<Commands::IUICommandTemplate>> commands);
        void registerCommand(std::shared_ptr<Commands::IUICommandTemplate> const &command);
        void registerMiddlware(std::shared_ptr<IUICommandMiddlware> const &middlware);

    signals:
        void actionsAvailable();
        void dispatched(int commandID, QVariant const &value);

    private slots:
        void onActionsAvailable();

    private:
        std::shared_ptr<UIAction> processAction(std::shared_ptr<UIAction> const &action);

    private:
        std::vector<std::shared_ptr<IUICommandMiddlware>> m_Middlwares;
        std::unordered_map<int, std::shared_ptr<Commands::IUICommandTemplate>> m_CommandsMap;
        QMutex m_Mutex;
        std::vector<std::shared_ptr<UIAction>> m_ActionsQueue;
        Commands::ICommandManager &m_CommandManager;
    };
}

#endif // UICOMMANDDISPATCHER_H

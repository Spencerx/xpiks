/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "uicommanddispatcher.h"
#include <QJSValue>
#include <QQmlEngine>
#include <Common/logging.h>
#include <Commands/commandmanager.h>
#include <Commands/Base/templateduicommand.h>
#include <Commands/UI/sourcetargetcommand.h>

namespace QMLExtensions {
    UICommandDispatcher::UICommandDispatcher(Commands::ICommandManager &commandManager, QObject *parent):
        QObject(parent),
        m_CommandManager(commandManager)
    {
        QObject::connect(this, &UICommandDispatcher::actionsAvailable,
                         this, &UICommandDispatcher::onActionsAvailable,
                         Qt::QueuedConnection);
    }

    void UICommandDispatcher::dispatch(int commandID, QJSValue const &value) {
        LOG_INFO << commandID;
        {
            QMutexLocker locker(&m_Mutex);
            Q_UNUSED(locker);
            m_ActionsQueue.push_back({commandID, value});
        }

        emit actionsAvailable();
    }

    QObject *UICommandDispatcher::getCommandTarget(int commandID) {
        LOG_INFO << commandID;
        auto it = m_CommandsMap.find(commandID);
        QObject *result = nullptr;
        Q_ASSERT(it != m_CommandsMap.end());
        if (it != m_CommandsMap.end()) {
            auto targetSource = std::dynamic_pointer_cast<Commands::UI::IUICommandTargetSource>(it->second);
            if (targetSource != nullptr) {
                QObject *target = targetSource->getTargetObject();
                Q_ASSERT(target != nullptr);
                if (target != nullptr) {
                    QQmlEngine::setObjectOwnership(target, QQmlEngine::CppOwnership);
                    result = target;
                }
            }
        } else {
            LOG_WARNING << "Command" << commandID << "not found";
        }
        return result;
    }

    void UICommandDispatcher::registerCommands(std::initializer_list<std::shared_ptr<Commands::IUICommandTemplate> > commands) {
        for (auto &command: commands) {
            registerCommand(command);
        }
    }

    void UICommandDispatcher::registerCommand(const std::shared_ptr<Commands::IUICommandTemplate> &command) {
        Q_ASSERT(command != nullptr);
        if (!command) { return; }
        const int commandID = command->getCommandID();
        Q_ASSERT(m_CommandsMap.find(commandID) == m_CommandsMap.end());
        m_CommandsMap[commandID] = command;
    }

#if defined(UI_TESTS) || defined(INTEGRATION_TESTS)
    void UICommandDispatcher::dispatchCommand(int commandID) {
        LOG_INFO << commandID;
        {
            QMutexLocker locker(&m_Mutex);
            Q_UNUSED(locker);
            m_ActionsQueue.push_back({commandID});
        }

        emit actionsAvailable();
    }
#endif

    void UICommandDispatcher::onActionsAvailable() {
        LOG_DEBUG << "#";
        decltype(m_ActionsQueue) actions;
        {
            QMutexLocker locker(&m_Mutex);
            Q_UNUSED(locker);
            m_ActionsQueue.swap(actions);
        }

        for (auto &action: actions) {
            processAction(action);
        }
    }

    void UICommandDispatcher::processAction(UIAction const &action) {
        LOG_INFO << action.m_CommandID << action.m_Value.toString();

        auto it = m_CommandsMap.find(action.m_CommandID);
        if (it != m_CommandsMap.end()) {
            m_CommandManager.processCommand(
                        std::make_shared<Commands::TemplatedUICommand>(action.m_Value.toVariant(),
                                                                       it->second));
        }

        emit dispatched(action.m_CommandID, action.m_Value);
    }
}

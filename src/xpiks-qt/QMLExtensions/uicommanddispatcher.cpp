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
    }

    void UICommandDispatcher::dispatch(int commandID, QJSValue const &value) {
        LOG_INFO << commandID;
        dispatchCommand(commandID, value.toVariant());
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

    void UICommandDispatcher::dispatchCommand(int commandID, QVariant const &value) {
        LOG_INFO << commandID << value.toString();
        auto it = m_CommandsMap.find(commandID);
        if (it != m_CommandsMap.end()) {
            m_CommandManager.processCommand(
                        std::make_shared<Commands::TemplatedUICommand>(value, it->second));
        }
    }
}

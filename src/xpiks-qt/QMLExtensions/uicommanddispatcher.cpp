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
#include <Common/logging.h>
#include <Commands/commandmanager.h>
#include <Commands/Base/templateduicommand.h>

namespace QMLExtensions {
    UICommandDispatcher::UICommandDispatcher(Commands::ICommandManager &commandManager, QObject *parent):
        QObject(parent),
        m_CommandManager(commandManager)
    {
    }

    void UICommandDispatcher::registerCommand(const std::shared_ptr<Commands::IUICommandTemplate> &command) {
        Q_ASSERT(command != nullptr);
        if (!command) { return; }
        const int commandID = command->getCommandID();
        Q_ASSERT(m_CommandsMap.find(commandID) == m_CommandsMap.end());
        m_CommandsMap[commandID] = command;
    }

    void UICommandDispatcher::dispatchCommand(int commandID, const QJSValue &value) {
        LOG_INFO << commandID << value;
        auto it = m_CommandsMap.find(commandID);
        if (it != m_CommandsMap.end()) {
            m_CommandManager.processCommand(
                        std::make_shared<Commands::TemplatedUICommand>(value, *it));
        }
    }
}

/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "uicommanddispatcher.h"

#include <utility>

#include <QJSValue>
#include <QMutexLocker>
#include <QQmlEngine>
#include <Qt>
#include <QtDebug>
#include <QtGlobal>

#include "Commands/Base/icommandmanager.h"
#include "Commands/Base/iuicommandtemplate.h"
#include "Commands/Base/templateduicommand.h"
#include "Commands/UI/sourcetargetcommand.h"
#include "Common/logging.h"
#include "QMLExtensions/iuicommandmiddleware.h"
#include "QMLExtensions/uiaction.h"
#include "QMLExtensions/uicommandid.h"

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
        QMLExtensions::UICommandID::CommandID command = static_cast<QMLExtensions::UICommandID::CommandID>(commandID);
        LOG_INFO << command;
        {
            QMutexLocker locker(&m_Mutex);
            Q_UNUSED(locker);
            m_ActionsQueue.emplace_back(std::make_shared<UIAction>(commandID, value.toVariant()));
        }

        emit actionsAvailable();
    }

    QObject *UICommandDispatcher::getCommandTarget(int commandID) {
        QMLExtensions::UICommandID::CommandID command = static_cast<QMLExtensions::UICommandID::CommandID>(commandID);
        LOG_INFO << command;
        auto it = m_CommandsMap.find(commandID);
        QObject *result = nullptr;
        Q_ASSERT(it != m_CommandsMap.end());
        if (it != m_CommandsMap.end()) {
            auto targetSource = std::dynamic_pointer_cast<Commands::UI::IUICommandTargetSource>(it->second);
            if (targetSource != nullptr) {
                QObject *target = targetSource->getTargetObject();
                Q_ASSERT(target != nullptr);
                LOG_INFO << "Found target from command" << command << commandID;
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

#if defined(INTEGRATION_TESTS) || defined(UI_TESTS)
    void UICommandDispatcher::dispatchCommand(int commandID, const QVariant &value) {
        LOG_DEBUG << "#";
        processAction(std::make_shared<UIAction>(commandID, value));
    }
#endif

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
        m_CommandsMap.emplace(commandID, command);
    }

    void UICommandDispatcher::registerMiddlware(std::shared_ptr<IUICommandMiddlware> const &middlware) {
        Q_ASSERT(middlware != nullptr);
        if (middlware != nullptr) {
            m_Middlwares.push_back(middlware);
        }
    }

    void UICommandDispatcher::onActionsAvailable() {
        LOG_DEBUG << "#";
        decltype(m_ActionsQueue) actions;
        {
            QMutexLocker locker(&m_Mutex);
            Q_UNUSED(locker);
            m_ActionsQueue.swap(actions);
        }

        for (auto &action: actions) {
            auto processedAction = processAction(action);
            emit dispatched(processedAction->m_CommandID, processedAction->m_Value);
        }
    }

    std::shared_ptr<UIAction> UICommandDispatcher::processAction(const std::shared_ptr<UIAction> &action) {
        QMLExtensions::UICommandID::CommandID commandID = static_cast<QMLExtensions::UICommandID::CommandID>(action->m_CommandID);
        LOG_INFO << commandID << action->m_Value.toString();

        std::shared_ptr<UIAction> actionToProcess = action;
        for (auto &middlware: m_Middlwares) {
            actionToProcess = middlware->process(actionToProcess);
        }

        auto it = m_CommandsMap.find(actionToProcess->m_CommandID);
        Q_ASSERT(it != m_CommandsMap.end());
        if (it != m_CommandsMap.end()) {
            m_CommandManager.processCommand(
                        std::make_shared<Commands::TemplatedUICommand>(actionToProcess->m_Value, it->second));
        }

        return actionToProcess;
    }
}

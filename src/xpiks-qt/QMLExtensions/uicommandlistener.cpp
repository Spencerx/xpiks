/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "uicommandlistener.h"
#include "uicommanddispatcher.h"

namespace QMLExtensions {
    UICommandListener::UICommandListener(QObject *parent):
        QObject(parent),
        m_Dispatcher(nullptr),
        m_IsEnabled(true)
    {
    }

    QObject *UICommandListener::getCommandDispatcher() const { return m_Dispatcher; }

    void UICommandListener::setCommandIDs(const QList<int> &commandIDs) {
        QSet<int> commandsSet = commandIDs.toSet();
        if (commandsSet != m_CommandIDs) {
            m_CommandIDs = commandsSet;
            emit commandIDsChanged();
        }
    }

    void UICommandListener::setCommandDispatcher(QObject *dispatcherObject) {
        if (dispatcherObject == nullptr) { return; }
        UICommandDispatcher *dispatcher = qobject_cast<UICommandDispatcher*>(dispatcherObject);
        if (dispatcher == nullptr) { return; }
        if (dispatcher == m_Dispatcher) { return; }

        disconnectDispatcher(m_Dispatcher);
        m_Dispatcher = dispatcher;
        QObject::connect(m_Dispatcher, &UICommandDispatcher::dispatched,
                         this, &UICommandListener::onDispatcherCommand);
        emit commandDispatcherChanged();
    }

    void UICommandListener::setIsEnabled(bool value) {
        if (m_IsEnabled != value) {
            m_IsEnabled = value;
            emit enabledChanged();
        }
    }

    void UICommandListener::disconnectDispatcher(UICommandDispatcher *dispatcher) {
        if (dispatcher != nullptr) {
            this->disconnect(dispatcher);
            dispatcher->disconnect(this);
        }
    }

    void UICommandListener::onDispatcherCommand(int commandID, QVariant const &value) {
        if (!m_IsEnabled) { return; }

        if (m_CommandIDs.empty() || m_CommandIDs.contains(commandID)) {
            emit dispatched(commandID, value);
        }
    }
}

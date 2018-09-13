/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef COMMANDLISTENER_H
#define COMMANDLISTENER_H

#include <QObject>
#include <QSet>
#include <QJSValue>

namespace QMLExtensions {
    class UICommandDispatcher;

    class UICommandListener : public QObject
    {
        Q_OBJECT
        Q_PROPERTY(QList<int> commandIDs READ getCommandIDs WRITE setCommandIDs NOTIFY commandIDsChanged)
        Q_PROPERTY(QObject *commandDispatcher READ getCommandDispatcher WRITE setCommandDispatcher NOTIFY commandDispatcherChanged)
        Q_PROPERTY(bool enabled READ getIsEnabled WRITE setIsEnabled NOTIFY enabledChanged)

    public:
        UICommandListener(QObject *parent=nullptr);

    public:
        QList<int> getCommandIDs() const { return m_CommandIDs.toList(); }
        QObject *getCommandDispatcher() const;
        bool getIsEnabled() const { return m_IsEnabled; }

    public:
        void setCommandIDs(const QList<int> &commandIDs);
        void setCommandDispatcher(QObject *dispatcherObject);
        void setIsEnabled(bool value);

    private:
        void disconnectDispatcher(UICommandDispatcher *dispatcher);

    signals:
        void commandIDsChanged();
        void commandDispatcherChanged();
        void enabledChanged();
        void dispatched(int commandID, QJSValue const &value);

    public slots:
        void onDispatcherCommand(int commandID, const QJSValue &value);

    private:
        QSet<int> m_CommandIDs;
        UICommandDispatcher *m_Dispatcher;
        bool m_IsEnabled;
    };
}

#endif // COMMANDLISTENER_H

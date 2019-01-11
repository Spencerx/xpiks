/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef UNDOREDOMANAGER_H
#define UNDOREDOMANAGER_H

#include <memory>
#include <stack>
#include <vector>

#include <QMutex>
#include <QObject>
#include <QString>

#include "Commands/Base/icommand.h"
#include "Common/messages.h"
#include "Common/types.h"
#include "UndoRedo/iundoredomanager.h"

namespace UndoRedo {
    class UndoRedoManager:
            public QObject,
            public IUndoRedoManager,
            public Common::MessagesTarget<Common::NamedType<int, Common::MessageType::UnavailableFiles>>
    {
        Q_OBJECT
        Q_PROPERTY(bool canUndo READ getCanUndo NOTIFY canUndoChanged)
        Q_PROPERTY(QString undoDescription READ getUndoDescription NOTIFY undoDescriptionChanged)
    public:
        UndoRedoManager(QObject *parent=nullptr):
            QObject(parent)
        { }

    public:
        bool getCanUndo() const { return !m_HistoryStack.empty(); }

    public:
        virtual void handleMessage(Common::NamedType<int, Common::MessageType::UnavailableFiles> const &message) override;

    signals:
        void canUndoChanged();
        void undoDescriptionChanged();
        void itemRecorded();
        void undoStackEmpty();

    private:
        QString getUndoDescription() const { return m_HistoryStack.empty() ? "" : m_HistoryStack.top()->getDescription(); }

    public:
        virtual void recordHistoryItem(const std::shared_ptr<Commands::ICommand> &historyItem) override;
        Q_INVOKABLE bool undoLastAction();
        Q_INVOKABLE void discardLastAction();

    private:
        std::shared_ptr<Commands::ICommand> popLastItem(bool &emptyNow);

    private:
        // stack for future todos
        std::stack<std::shared_ptr<Commands::ICommand>> m_HistoryStack;
        QMutex m_Mutex;
    };
}

#endif // UNDOREDOMANAGER_H

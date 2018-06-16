/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef COMMANDMANAGER_H
#define COMMANDMANAGER_H

#include "icommandmanager.h"

namespace UndoRedo {
    class UndoRedoManager;
}

namespace Commands {
    class CommandManager : public ICommandManager
    {
    public:
        CommandManager(UndoRedo::UndoRedoManager &undoRedoManager);

    private:
        int generateNextCommandID() { int id = m_LastCommandID++; return id; }

    public:
        virtual std::shared_ptr<Commands::CommandResult> processCommand(const std::shared_ptr<IAppCommand> &command) override;

    private:
        volatile int m_LastCommandID;
        UndoRedo::UndoRedoManager &m_UndoRedoManager;
    };
}

#endif // COMMANDMANAGER_H

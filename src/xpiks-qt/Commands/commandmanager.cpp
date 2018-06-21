/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "commandmanager.h"
#include "icommand.h"
#include "../UndoRedo/undoredomanager.h"

namespace Commands {
    CommandManager::CommandManager(UndoRedo::UndoRedoManager &undoRedoManager):
        m_UndoRedoManager(undoRedoManager),
        m_LastCommandID(0)
    {
    }

    std::shared_ptr<Commands::CommandResult> CommandManager::processCommand(const std::shared_ptr<Commands::IAppCommand> &command) {
        Q_ASSERT(command);
        const int id = generateNextCommandID();
        std::shared_ptr<Commands::CommandResult> result = command->execute(id);
        auto undoCommand = std::dynamic_pointer_cast<Commands::IUndoCommand>(command);
        if (undoCommand) {
            m_UndoRedoManager.recordHistoryItem(undoCommand);
        }
        return result;
    }
}

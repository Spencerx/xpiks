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

    void CommandManager::processCommand(const std::shared_ptr<ICommand> &command) {
        Q_ASSERT(command);
        command->execute();
        if (command->canUndo()) {
            m_UndoRedoManager.recordHistoryItem(command);
        }
        return result;
    }
}

#ifndef COMMANDMANAGERMOCK_H
#define COMMANDMANAGERMOCK_H

#include <memory>

#include "Commands/commandmanager.h"

namespace Commands {
    class ICommand;
}

namespace UndoRedo {
    class IUndoRedoManager;
}

namespace Mocks {
    class CommandManagerMock : public Commands::CommandManager
    {
    public:
        CommandManagerMock(UndoRedo::IUndoRedoManager &undoRedoManager):
            Commands::CommandManager(undoRedoManager),
            m_AnyCommandProcessed(false),
            m_CanExecuteCommands(true)
        {
        }

    public:
        bool anyCommandProcessed() const { return m_AnyCommandProcessed; }
        void resetAnyCommandProcessed() { m_AnyCommandProcessed = false; }
        void disableCommands() { m_CanExecuteCommands = false; }
        void enableCommands() { m_CanExecuteCommands = true; }

    public:
        virtual void processCommand(const std::shared_ptr<Commands::ICommand> &command) override {
            m_AnyCommandProcessed = true;
            if (m_CanExecuteCommands) {
                Commands::CommandManager::processCommand(command);
            }
        }

    private:
        bool m_AnyCommandProcessed;
        bool m_CanExecuteCommands;
    };
}

#endif // COMMANDMANAGERMOCK_H

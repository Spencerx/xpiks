#ifndef COMMANDMANAGERMOCK_H
#define COMMANDMANAGERMOCK_H

#include <Commands/commandmanager.h>

namespace Mocks {
    class CommandManagerMock : public Commands::CommandManager
    {
    public:
        CommandManagerMock():
            m_AnyCommandProcessed(false),
            m_CanExecuteCommands(true)
        {
            m_TestsDelegator.setCommandManager(this);
            this->InjectDependency(&m_TestsDelegator);
        }

    public:
        bool anyCommandProcessed() const { return m_AnyCommandProcessed; }
        void resetAnyCommandProcessed() { m_AnyCommandProcessed = false; }
        void disableCommands() { m_CanExecuteCommands = false; }
        void enableCommands() { m_CanExecuteCommands = true; }
        void mockAcceptDeletion() { getDelegator()->removeUnavailableFiles();}

    public:


        virtual std::shared_ptr<Commands::ICommandResult> processCommand(const std::shared_ptr<Commands::ICommandBase> &command) {
            m_AnyCommandProcessed = true;
            if (m_CanExecuteCommands) {
                return Commands::CommandManager::processCommand(command);
            } else {
                return std::shared_ptr<Commands::ICommandResult>();
            }
        }

        void mockDeletion(int count) {
            for (int i = 0; i < count; ++i) {
                CommandManager::getArtItemsModel()->getArtwork(i)->setUnavailable();
            }
        }

    private:
        Commands::MainDelegator m_TestsDelegator;
        bool m_AnyCommandProcessed;
        volatile bool m_CanExecuteCommands;
    };
}

#endif // COMMANDMANAGERMOCK_H

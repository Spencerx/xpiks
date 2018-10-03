/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef CURRENTEDITABLECOMMANDS_H
#define CURRENTEDITABLECOMMANDS_H

#include <Commands/Base/iuicommandtemplate.h>
#include <QMLExtensions/uicommandid.h>
#include "sourcetargetcommand.h"

namespace Models {
    class ICurrentEditableSource;
}

namespace Commands {
    class ICommandManager;
}

namespace Commands {
    namespace UI {
        class CurrentEditableCommand: public IUICommandTemplate {
        public:
            CurrentEditableCommand(int commandID,
                                   Models::ICurrentEditableSource &currentEditableSource,
                                   Commands::ICommandManager &commandManager):
                m_CommandID(commandID),
                m_CurrentEditableSource(currentEditableSource),
                m_CommandManager(commandManager)
            { }

            // IUICommandTemplate interface
        public:
            virtual int getCommandID() override { return m_CommandID; }

        private:
            int m_CommandID;

        protected:
            Models::ICurrentEditableSource &m_CurrentEditableSource;
            Commands::ICommandManager &m_CommandManager;
        };

        class PlainTextEditCommand: public CurrentEditableCommand {
        public:
            PlainTextEditCommand(Models::ICurrentEditableSource &currentEditableSource,
                                 Commands::ICommandManager &commandManager):
                CurrentEditableCommand(QMLExtensions::UICommandID::PlainTextEdit,
                                       currentEditableSource,
                                       commandManager)
            { }

            // IUICommandTemplate interface
        public:
            virtual void execute(const QVariant &value) override;
        };

        class AppendSuggestedKeywordsCommand: public CurrentEditableCommand {
        public:
            AppendSuggestedKeywordsCommand(Models::ICurrentEditableSource &currentEditableSource,
                                           Commands::ICommandManager &commandManager):
                CurrentEditableCommand(QMLExtensions::UICommandID::AppendSuggestedKeywords,
                                       currentEditableSource,
                                       commandManager)
            { }

            // IUICommandTemplate interface
        public:
            virtual void execute(const QVariant &value) override;
        };
    }
}

#endif // CURRENTEDITABLECOMMANDS_H

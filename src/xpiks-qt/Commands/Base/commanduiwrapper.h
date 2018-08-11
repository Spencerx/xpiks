/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef COMMANDUIWRAPPER_H
#define COMMANDUIWRAPPER_H

#include "iuicommandtemplate.h"
#include "icommand.h"
#include <memory>

namespace Commands {
    class CommandUIWrapper: public IUICommandTemplate {
    public:
        CommandUIWrapper(int ID, std::shared_ptr<ICommand> const &command):
            m_ID(ID),
            m_Command(command)
        {
            Q_ASSERT(m_Command != nullptr);
        }

        // IUICommandTemplate interface
    public:
        virtual int getCommandID() override { return m_ID; }
        virtual void execute(QVariant const &) override { m_Command->execute(); }
        virtual void undo(QVariant const &) override { m_Command->undo(); }
        virtual bool canUndo() override { return m_Command->canUndo(); }
        virtual QString getDescription() const override { return m_Command->getDescription(); }

    private:
        int m_ID;
        std::shared_ptr<ICommand> m_Command;
    };
}

#endif // COMMANDUIWRAPPER_H

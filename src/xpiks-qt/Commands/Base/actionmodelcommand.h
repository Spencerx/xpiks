/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef ACTIONMODELCOMMAND_H
#define ACTIONMODELCOMMAND_H

#include "iuicommandtemplate.h"
#include "icommandtemplate.h"
#include <memory>

namespace Models {
    class IActionModel;
}

namespace Commands {
    class ICommand;

    class ActionModelCommand : public IUICommandTemplate
    {
    public:
        ActionModelCommand(Models::IActionModel &model, int commandID);

        // IUICommandTemplate interface
    public:
        virtual int getCommandID() override { return m_CommandID; }
        virtual void execute(const QVariant &value) override;
        virtual void undo(const QVariant &) override;
        virtual bool canUndo() override;
        virtual QString getDescription() const override;

    private:
        Models::IActionModel &m_Model;
        std::shared_ptr<ICommand> m_Command;
        int m_CommandID;
    };
}

#endif // ACTIONMODELCOMMAND_H

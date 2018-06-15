/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef ICOMMANDBASE_H
#define ICOMMANDBASE_H

#include <memory>

namespace Commands {
    class ICommandManager;

    class CommandResult {
    public:
        virtual ~CommandResult() {}
        virtual int getStatus() const { return 0; }
    };

    class ICommand {
    public:
        virtual ~ICommand() {}
        virtual std::shared_ptr<CommandResult> execute() { return std::make_shared(new CommandResult()); }
    };

    class IAppCommand: public ICommand {
    public:
        virtual ~IAppCommand() {}
        virtual std::shared_ptr<CommandResult> execute(int commandID) {
            Q_UNUSED(commandID);
            return ICommand::execute();
        }
    };
}

#endif // ICOMMANDBASE_H

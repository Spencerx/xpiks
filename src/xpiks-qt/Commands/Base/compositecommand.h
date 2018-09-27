/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef COMPOSITECOMMAND_H
#define COMPOSITECOMMAND_H

#include "icommand.h"
#include <memory>
#include <vector>

namespace Commands {
    class CompositeCommand: public ICommand
    {
    public:
        CompositeCommand(std::initializer_list<std::shared_ptr<ICommand>> list):
            m_Commands(list)
        {}

        CompositeCommand(std::vector<std::shared_ptr<ICommand>> &commands):
            m_Commands(std::move(commands))
        {}

        // ICommand interface
    public:
        virtual void execute() override;
        virtual void undo() override;
        virtual bool canUndo() override;

    private:
        std::vector<std::shared_ptr<ICommand>> m_Commands;
    };
}

#endif // COMPOSITECOMMAND_H

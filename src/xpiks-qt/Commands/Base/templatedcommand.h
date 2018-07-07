/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef TEMPLATEDCOMMAND_H
#define TEMPLATEDCOMMAND_H

#include "icommand.h"
#include "icommandtemplate.h"
#include <Artworks/artworkssnapshot.h>
#include <memory>
#include <type_traits>

namespace Artworks {
    class ArtworkMetadata;
}

namespace Commands {
    template<typename T>
    class TemplatedCommand: public ICommand {
        static_assert(std::is_move_constructible<T>::value, "T should have move constructor");
    public:
        TemplatedCommand(const T &argument, const std::shared_ptr<ICommandTemplate<T>> &commandTemplate):
            m_Argument(argument),
            m_CommandTemplate(commandTemplate)
        {
            Q_ASSERT(m_CommandTemplate);
        }

        // ICommand interface
    public:
        virtual void execute() override { m_CommandTemplate->execute(m_Argument); }
        virtual void undo() override { m_CommandTemplate->undo(m_Argument); }

    protected:
        const T &m_Argument;
        std::shared_ptr<ICommandTemplate<T>> m_CommandTemplate;
    };
}

#endif // TEMPLATEDCOMMAND_H

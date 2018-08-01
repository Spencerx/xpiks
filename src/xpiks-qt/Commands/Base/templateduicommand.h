/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef TEMPLATEDUICOMMAND_H
#define TEMPLATEDUICOMMAND_H

#include <QJSValue>
#include <memory>
#include "iuicommandtemplate.h"
#include "icommand.h"

namespace Commands {
    class TemplatedUICommand: public ICommand
    {
    public:
        TemplatedUICommand(const QJSValue &value,
                           const std::shared_ptr<IUICommandTemplate> &commandTemplate):
            m_Value(value),
            m_CommandTemplate(commandTemplate)
        { }

        // ICommand interface
    public:
        virtual void execute() override { m_CommandTemplate->execute(m_Value); }
        virtual void undo() override { m_CommandTemplate->undo(); }
        virtual bool canUndo() override { return m_CommandTemplate->canUndo(); }
        virtual QString getDescription() const override { return m_CommandTemplate->getDescription(); }

    private:
        QJSValue m_Value;
        std::shared_ptr<IUICommandTemplate> m_CommandTemplate;
    };
}

#endif // TEMPLATEDUICOMMAND_H

/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "actionmodelcommand.h"

#include <QtDebug>
#include <QtGlobal>

#include "Commands/Base/icommand.h"
#include "Common/logging.h"
#include "Helpers/uihelpers.h"
#include "Models/iactionmodel.h"

namespace Commands {
    ActionModelCommand::ActionModelCommand(Models::IActionModel &model, int commandID):
        m_Model(model),
        m_CommandID(commandID)
    {
    }

    void ActionModelCommand::execute(const QVariant &value) {
        LOG_DEBUG << "#";
        bool yesno = Helpers::convertToBool(value, false);
        m_Command = m_Model.getActionCommand(yesno);
        Q_ASSERT(m_Command != nullptr);
        m_Command->execute();
    }

    void ActionModelCommand::undo(const QVariant &) {
        Q_ASSERT(m_Command != nullptr);
        if (m_Command != nullptr) {
            m_Command->undo();
        }
    }

    bool ActionModelCommand::canUndo() {
        Q_ASSERT(m_Command != nullptr);
        return (m_Command != nullptr) && (m_Command->canUndo());
    }

    QString ActionModelCommand::getDescription() const {
        Q_ASSERT(m_Command != nullptr);
        if (m_Command != nullptr) {
            return m_Command->getDescription();
        } else {
            return QString();
        }
    }
}

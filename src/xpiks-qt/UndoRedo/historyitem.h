/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef HISTORYITEM_H
#define HISTORYITEM_H

#include "ihistoryitem.h"

namespace UndoRedo {
    class HistoryItem : public IHistoryItem
    {
        enum {
            UNKNOWN_COMMAND_ID = -1
        };

    public:
        HistoryItem(int commandID=UNKNOWN_COMMAND_ID):
            m_CommandID(commandID)
        { }
        virtual ~HistoryItem() {}

    public:
        virtual int getCommandID() const override { return m_CommandID; }

    private:
        int m_CommandID;
    };
}

#endif // HISTORYITEM_H

/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef ADDARTWORKITEM_H
#define ADDARTWORKITEM_H

#include <QObject>
#include <QVector>
#include <QPair>
#include "historyitem.h"
#include "../Helpers/indiceshelper.h"
#include "../Common/defines.h"

namespace UndoRedo {
   class AddArtworksHistoryItem : public HistoryItem
    {
    public:
        AddArtworksHistoryItem(int commandID, int firstIndex, int count) :
           HistoryItem(HistoryActionType::AddArtworks, commandID)
        {
            Q_ASSERT(count > 0);
            m_AddedRanges.append(qMakePair(firstIndex, firstIndex + count - 1));
        }

        AddArtworksHistoryItem(int commandID, const QVector<QPair<int, int> > &rangesAdded) :
           HistoryItem(HistoryActionType::AddArtworks, commandID),
           m_AddedRanges(rangesAdded)
        {
            Q_ASSERT(!rangesAdded.empty());
        }

       virtual ~AddArtworksHistoryItem() { LOG_DEBUG << "#"; }

   public:
        virtual void undo(const Commands::ICommandManager *commandManagerInterface) override;

   public:
        virtual QString getDescription() const override {
            int count = Helpers::getRangesLength(m_AddedRanges);
            return count > 1 ? QObject::tr("%1 items added").arg(count) :
                                 QObject::tr("1 item added");
        }

    private:
        QVector<QPair<int, int> > m_AddedRanges;
    };
}

#endif // ADDARTWORKITEM_H

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
#include "../Common/logging.h"
#include "../Commands/icommand.h"

namespace Models {
    class ArtItemsModel;
}

namespace UndoRedo {
   class AddArtworksHistoryItem : public HistoryItem
    {
    public:
        AddArtworksHistoryItem(Models::ArtItemsModel &artItemsModel, int firstIndex, int count,
                               const std::shared_ptr<Commands::ICommand> &saveSessionCommand) :
           HistoryItem(),
           m_ArtItemsModel(artItemsModel),
           m_SaveSessionCommand(saveSessionCommand)
        {
            Q_ASSERT(count > 0);
            m_AddedRanges.append(qMakePair(firstIndex, firstIndex + count - 1));
        }

        AddArtworksHistoryItem(Models::ArtItemsModel &artItemsModel, const QVector<QPair<int, int> > &rangesAdded) :
           HistoryItem(),
           m_AddedRanges(rangesAdded),
           m_ArtItemsModel(artItemsModel)
        {
            Q_ASSERT(!rangesAdded.empty());
        }

       virtual ~AddArtworksHistoryItem() { LOG_DEBUG << "#"; }

   public:
        virtual void undo() override;

   public:
        virtual QString getDescription() const override {
            int count = Helpers::getRangesLength(m_AddedRanges);
            return count > 1 ? QObject::tr("%1 items added").arg(count) :
                                 QObject::tr("1 item added");
        }

    private:
        QVector<QPair<int, int> > m_AddedRanges;
        Models::ArtItemsModel &m_ArtItemsModel;
        std::shared_ptr<Commands::ICommand> m_SaveSessionCommand;
    };
}

#endif // ADDARTWORKITEM_H

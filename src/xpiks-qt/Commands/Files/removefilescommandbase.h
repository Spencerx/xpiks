/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef REMOVEFILESCOMMAND_H
#define REMOVEFILESCOMMAND_H

#include <Commands/Base/icommand.h>
#include <Models/Artworks/artworkslistoperations.h>

namespace Models {
    class ArtworksRepository;
    class ArtworksListModel;
}

namespace Commands {
    class RemoveFilesCommandBase: public ICommand
    {
    public:
        RemoveFilesCommandBase(Models::ArtworksListModel &artworksList,
                               Models::ArtworksRepository &artworksRepository);

        // IUndoCommand interface
    public:
        virtual bool canUndo() override { return true; }
        virtual void undo() override;
        virtual QString getDescription() const override {
            return m_RemoveResult.m_RemovedCount != 1 ?
                        QObject::tr("%1 items removed").arg(m_RemoveResult.m_RemovedCount) :
                        QObject::tr("1 item removed");
        }

    public:
        int getRemovedCount() const { return m_RemoveResult.m_RemovedCount; }

    protected:
        Models::ArtworksListModel &m_ArtworksList;
        Models::ArtworksRepository &m_ArtworksRepository;
        Models::ArtworksRemoveResult m_RemoveResult;
    };
}

#endif // REMOVEFILESCOMMAND_H

/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef REMOVEFILESCOMMANDBASE_H
#define REMOVEFILESCOMMANDBASE_H

#include <cstddef>
#include <memory>

#include <QObject>
#include <QString>

#include "Commands/Base/icommand.h"
#include "Models/Artworks/artworkslistoperations.h"

namespace Models {
    class ArtworksRepository;
    class ArtworksListModel;
}

namespace Commands {
    class ICommand;

    class RemoveFilesCommandBase: public ICommand
    {
    public:
        RemoveFilesCommandBase(Models::ArtworksListModel &artworksList,
                               Models::ArtworksRepository &artworksRepository,
                               std::shared_ptr<ICommand> const &saveSessionCommand =
                std::shared_ptr<ICommand>());

        // IUndoCommand interface
    public:
        virtual void execute() override;
        virtual bool canUndo() override { return true; }
        virtual void undo() override;
        virtual QString getDescription() const override {
            return m_RemoveResult.m_RemovedCount != 1 ?
                        QObject::tr("%1 items removed").arg(m_RemoveResult.m_RemovedCount) :
                        QObject::tr("1 item removed");
        }

    public:
        size_t getRemovedCount() const { return m_RemoveResult.m_RemovedCount; }

    protected:
        virtual void restoreFiles();
        virtual Models::ArtworksRemoveResult removeFiles() = 0;

    protected:
        Models::ArtworksListModel &m_ArtworksList;
        Models::ArtworksRepository &m_ArtworksRepository;
        Models::ArtworksRemoveResult m_RemoveResult;
        std::shared_ptr<ICommand> m_SaveSessionCommand;
    };
}

#endif // REMOVEFILESCOMMANDBASE_H

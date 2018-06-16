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

#include "icommand.h"
#include "../Models/artitemsmodel.h"
#include "../Filesystem/ifilescollection.h"

namespace Commands {
    class RemoveFilesCommand: public IUndoCommand
    {
    public:
        RemoveFilesCommand();

    protected:
        virtual Models::ArtItemsModel::ArtworksRemoveResult removeFiles() = 0;

        // IAppCommand interface
    public:
        virtual std::shared_ptr<CommandResult> execute(int commandID) override;

        // IUndoCommand interface
    public:
        virtual void undo() override;
        virtual QString getDescription() const override;
        virtual int getCommandID() const override { return m_CommandID; }

    private:

        int m_CommandID = 0;
        int m_RemovedCount = 0;
    };
}

#endif // REMOVEFILESCOMMAND_H

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

#include <memory>

#include "Commands/Files/removefilescommandbase.h"
#include "Helpers/indicesranges.h"
#include "Models/Artworks/artworkslistoperations.h"

namespace Models {
    class ArtworksListModel;
    class ArtworksRepository;
}

namespace Commands {
    class ICommand;

    class RemoveFilesCommand: public RemoveFilesCommandBase
    {
    public:
        RemoveFilesCommand(Helpers::IndicesRanges const &ranges,
                           Models::ArtworksListModel &artworksList,
                           Models::ArtworksRepository &artworksRepository,
                           std::shared_ptr<ICommand> const &saveSessionCommand =
                std::shared_ptr<ICommand>());

        // ICommand interface
    public:
        virtual void execute() override;
        virtual void undo() override;

        // RemoveFilesCommandBase interface
    protected:
        virtual Models::ArtworksRemoveResult removeFiles() override;
        virtual void setRangesToRemove(Helpers::IndicesRanges const &ranges) { m_Ranges = ranges; }

    private:
        Helpers::IndicesRanges m_Ranges;
        std::shared_ptr<ICommand> m_SaveSessionCommand;
    };
}

#endif // REMOVEFILESCOMMAND_H

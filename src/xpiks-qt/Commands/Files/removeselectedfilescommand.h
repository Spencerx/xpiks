/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef REMOVESELECTEDFILESCOMMAND_H
#define REMOVESELECTEDFILESCOMMAND_H

#include <memory>

#include "Commands/Files/removefilescommand.h"

namespace Models {
    class ArtworksListModel;
    class ArtworksRepository;
}

namespace Artworks {
    class ISelectedIndicesSource;
}

namespace Commands {
    class ICommand;

    class RemoveSelectedFilesCommand: public RemoveFilesCommand
    {
    public:
        RemoveSelectedFilesCommand(Artworks::ISelectedIndicesSource &indicesSource,
                                   Models::ArtworksListModel &artworksList,
                                   Models::ArtworksRepository &artworksRepository,
                                   std::shared_ptr<ICommand> const &saveSessionCommand =
                std::shared_ptr<ICommand>());

        // ICommand interface
    public:
        virtual void execute() override;

    private:
        Artworks::ISelectedIndicesSource &m_IndicesSource;
    };
}

#endif // REMOVESELECTEDFILESCOMMAND_H

/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef REMOVESELECTEDFILESCOMMAND_H
#define REMOVESELECTEDFILESCOMMAND_H

#include "removefilescommandbase.h"

namespace Artworks {
    class ISelectedIndicesSource;
}

namespace Commands {
    class RemoveSelectedFilesCommand: public RemoveFilesCommandBase
    {
    public:
        RemoveSelectedFilesCommand(Artworks::ISelectedIndicesSource &indicesSource,
                                   Models::ArtworksListModel &artworksList,
                                   Models::ArtworksRepository &artworksRepository,
                                   std::shared_ptr<ICommand> const &saveSessionCommand);

        // ICommand interface
    public:
        virtual void execute() override;
        virtual void undo() override;

    private:
        Artworks::ISelectedIndicesSource &m_IndicesSource;
        std::shared_ptr<ICommand> m_SaveSessionCommand;
    };
}

#endif // REMOVESELECTEDFILESCOMMAND_H

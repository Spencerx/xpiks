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
#include <Helpers/indiceshelper.h>

namespace Commands {
    class RemoveFilesCommand: public RemoveFilesCommandBase
    {
    public:
        RemoveFilesCommand(Helpers::IndicesRanges &ranges,
                           Models::ArtworksListModel &artworksList,
                           Models::ArtworksRepository &artworksRepository);

        // ICommand interface
    public:
        virtual void execute() override;

    private:
        Helpers::IndicesRanges m_Ranges;
    };
}

#endif // REMOVESELECTEDFILESCOMMAND_H

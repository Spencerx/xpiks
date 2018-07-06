/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "removefilescommand.h"
#include <Models/Artworks/artworkslistmodel.h>
#include <Models/Artworks/artworksrepository.h>

namespace Commands {
    RemoveFilesCommand::RemoveFilesCommand(Helpers::IndicesRanges &ranges,
                                           Models::ArtworksListModel &artworksList,
                                           Models::ArtworksRepository &artworksRepository):
        RemoveFilesCommandBase(artworksList, artworksRepository),
        m_Ranges(std::move(ranges))
    {
    }

    void RemoveFilesCommand::execute() {
        m_RemoveResult = m_ArtworksList.removeFiles(m_Ranges);
    }
}

/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "removeselectedfilescommand.h"

#include <QtDebug>

#include "Artworks/iselectedindicessource.h"
#include "Commands/Files/removefilescommand.h"
#include "Common/logging.h"
#include "Helpers/indicesranges.h"

namespace Commands {
    RemoveSelectedFilesCommand::RemoveSelectedFilesCommand(Artworks::ISelectedIndicesSource &indicesSource,
                                                           Models::ArtworksListModel &artworksList,
                                                           Models::ArtworksRepository &artworksRepository,
                                                           const std::shared_ptr<Commands::ICommand> &saveSessionCommand):
        RemoveFilesCommand(Helpers::IndicesRanges(),
                           artworksList,
                           artworksRepository,
                           saveSessionCommand),
        m_IndicesSource(indicesSource)
    {
    }

    void RemoveSelectedFilesCommand::execute() {
        LOG_DEBUG << "#";
        Helpers::IndicesRanges ranges(m_IndicesSource.getSelectedIndices());
        setRangesToRemove(ranges);

        RemoveFilesCommand::execute();
    }
}

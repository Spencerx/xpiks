/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef ADDTORECENTTEMPLATE_H
#define ADDTORECENTTEMPLATE_H

#include <Commands/Base/icommandtemplate.h>

namespace Artworks {
    class ArtworksSnapshot;
}

namespace Models {
    class RecentFilesModel;
}

namespace Commands {
    class AddToRecentTemplate: public ICommandTemplate<Artworks::ArtworksSnapshot>
    {
    public:
        AddToRecentTemplate(Models::RecentFilesModel &recentFilesModel);

        // IArtworksCommandTemplate interface
    public:
        virtual void execute(const Artworks::ArtworksSnapshot &snapshot) override;

    private:
        Models::RecentFilesModel &m_RecentFilesModel;
    };
}

#endif // ADDTORECENTTEMPLATE_H

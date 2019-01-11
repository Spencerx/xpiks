/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef CLEANUPLEGACYBACKUPSCOMMAND_H
#define CLEANUPLEGACYBACKUPSCOMMAND_H

#include <memory>

#include <QStringList>

#include "Commands/Base/icommand.h"
#include "Commands/Base/icommandtemplate.h"

namespace Filesystem {
    class IFilesCollection;
}

namespace Maintenance {
    class MaintenanceService;
}

namespace Artworks {
    class ArtworksSnapshot;
}

namespace Commands {
    class CleanupLegacyBackupsCommand: public ICommand, public ICommandTemplate<Artworks::ArtworksSnapshot>
    {
    public:
        CleanupLegacyBackupsCommand(const std::shared_ptr<Filesystem::IFilesCollection> &files,
                                    Maintenance::MaintenanceService &maintenanceService);

        // ICommand interface
    public:
        virtual void execute() override;

        // IArtworksCommandTemplate interface
    public:
        virtual void execute(const Artworks::ArtworksSnapshot &) override { execute(); }

    public:
        QStringList m_Directories;
        Maintenance::MaintenanceService &m_MaintenanceService;
    };
}

#endif // CLEANUPLEGACYBACKUPSCOMMAND_H

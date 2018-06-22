/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef CLEANUPLEGACYBACKUPSCOMMAND_H
#define CLEANUPLEGACYBACKUPSCOMMAND_H

#include <memory>
#include <QStringList>
#include "icommand.h"
#include "../Filesystem/ifilescollection.h"
#include "iartworkscommandtemplate.h"

namespace Maintenance {
    class MaintenanceService;
}

namespace Commands {
    class CleanupLegacyBackupsCommand: public ICommand, public IArtworksCommandTemplate
    {
    public:
        CleanupLegacyBackupsCommand(const std::shared_ptr<Filesystem::IFilesCollection> &files,
                                    Maintenance::MaintenanceService &maintenanceService);

        // ICommand interface
    public:
        virtual void execute() override;

        // IArtworksCommandTemplate interface
    public:
        virtual void execute(Artworks::ArtworksSnapshot &) override { execute(); }

    public:
        QStringList m_Directories;
        Maintenance::MaintenanceService &m_MaintenanceService;
    };
}

#endif // CLEANUPLEGACYBACKUPSCOMMAND_H

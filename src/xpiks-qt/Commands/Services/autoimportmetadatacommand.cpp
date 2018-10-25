/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "autoimportmetadatacommand.h"

#include <QtDebug>

#include "Artworks/artworkssnapshot.h"
#include "Common/logging.h"
#include "MetadataIO/metadataiocoordinator.h"
#include "Models/settingsmodel.h"
#include "Models/switchermodel.h"

namespace Commands {
    AutoImportMetadataCommand::AutoImportMetadataCommand(MetadataIO::MetadataIOCoordinator &metadataIOCoordinator,
                                                         Models::SettingsModel &settingsModel,
                                                         Models::SwitcherModel &switcherModel,
                                                         bool ignoreBackups):
        m_MetadataIOCoordinator(metadataIOCoordinator),
        m_SettingsModel(settingsModel),
        m_SwitcherModel(switcherModel),
        m_IgnoreBackups(ignoreBackups)
    {
    }

    void AutoImportMetadataCommand::execute() {
        LOG_DEBUG << "#";
        if (m_SettingsModel.getUseAutoImport() && m_SwitcherModel.getUseAutoImport()) {
            LOG_DEBUG << "Autoimport is ON. Proceeding...";
            m_MetadataIOCoordinator.continueReading(m_IgnoreBackups);
        }
    }
}

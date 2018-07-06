/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef AUTOIMPORTMETADATACOMMAND_H
#define AUTOIMPORTMETADATACOMMAND_H

#include <Commands/Base/icommand.h>

namespace MetadataIO {
    class MetadataIOCoordinator;
}

namespace Models {
    class SettingsModel;
    class SwitcherModel;
}

namespace Commands {
    class AutoImportMetadataCommand: public ICommand
    {
    public:
        AutoImportMetadataCommand(MetadataIO::MetadataIOCoordinator &metadataIOCoordinator,
                                  Models::SettingsModel &settingsModel,
                                  Models::SwitcherModel &switcherModel);

        // ICommand interface
    public:
        virtual void execute() override;

    private:
        MetadataIO::MetadataIOCoordinator &m_MetadataIOCoordinator;
        Models::SettingsModel &m_SettingsModel;
        Models::SwitcherModel &m_SwitcherModel;
    };
}

#endif // AUTOIMPORTMETADATACOMMAND_H

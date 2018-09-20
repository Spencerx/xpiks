/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef GENERALCOMMANDS_H
#define GENERALCOMMANDS_H

#include <Commands/Base/iuicommandtemplate.h>
#include <QMLExtensions/uicommandid.h>
#include "sourcetargetcommand.h"

namespace Models {
    class SettingsModel;
    class ArtworksListModel;
    class FilteredArtworksListModel;
    class UploadInfoRepository;
    class LogsModel;
}

namespace Encryption {
    class SecretsManager;
}

namespace AutoComplete {
    class AutoCompleteService;
}

namespace Warnings {
    class WarningsModel;
}

namespace Commands {
    namespace UI {
        SOURCE_TARGET_COMMAND(SetMasterPasswordCommand,
                              QMLExtensions::UICommandID::SetMasterPassword,
                              Encryption::SecretsManager,
                              Models::SettingsModel);

        SOURCE_COMMAND(RemoveUnavailableFilesCommand,
                       QMLExtensions::UICommandID::RemoveUnavailableFiles,
                       Models::ArtworksListModel);

        SOURCE_COMMAND(GenerateCompletionsCommand,
                       QMLExtensions::UICommandID::GenerateCompletions,
                       AutoComplete::AutoCompleteService);

        SOURCE_COMMAND(SelectFilteredArtworksCommand,
                       QMLExtensions::UICommandID::SelectFilteredArtworks,
                       Models::FilteredArtworksListModel);

        SOURCE_COMMAND(CheckWarningsCommand,
                       QMLExtensions::UICommandID::CheckWarnings,
                       Warnings::WarningsModel);

        SOURCE_COMMAND(InitUploadHostCommand,
                       QMLExtensions::UICommandID::InitUploadHosts,
                       Models::UploadInfoRepository);

        TARGET_COMMAND(UpdateLogsCommand,
                       QMLExtensions::UICommandID::UpdateLogs,
                       Models::LogsModel);
    }
}

#endif // GENERALCOMMANDS_H

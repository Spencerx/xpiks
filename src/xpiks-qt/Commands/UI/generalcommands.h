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
}

namespace Encryption {
    class SecretsManager;
}

namespace Commands {
    namespace UI {
        SOURCE_TARGET_COMMAND(SetMasterPasswordCommand,
                              QMLExtensions::UICommandID::SetMasterPassword,
                              Encryption::SecretsManager,
                              Models::SettingsModel);

        class RemoveUnavailableFilesCommand: public IUICommandTemplate {
        public:
            RemoveUnavailableFilesCommand(Models::ArtworksListModel &artworksListModel):
                m_ArtworksListModel(artworksListModel)
            {}

            // IUICommandTemplate interface
        public:
            virtual int getCommandID() override { return QMLExtensions::UICommandID::RemoveUnavailableFiles; }
            virtual void execute(const QJSValue &value) override;

        private:
            Models::ArtworksListModel &m_ArtworksListModel;
        };
    }
}

#endif // GENERALCOMMANDS_H

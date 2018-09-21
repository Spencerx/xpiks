/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "generalcommands.h"
#include <Models/settingsmodel.h>
#include <Encryption/secretsmanager.h>
#include <Models/Artworks/artworkslistmodel.h>
#include <Models/Artworks/filteredartworkslistmodel.h>
#include <Models/Connectivity/uploadinforepository.h>
#include <Models/logsmodel.h>
#include <Services/AutoComplete/autocompleteservice.h>
#include <Services/Warnings/warningsmodel.h>
#include <Services/SpellCheck/userdicteditmodel.h>
#include <Helpers/uihelpers.h>

namespace Commands {
    namespace UI {
        void SetMasterPasswordCommand::execute(QVariant const &) {
            LOG_DEBUG << "#";
            m_Target.onMasterPasswordSet(m_Source);
        }

        void RemoveUnavailableFilesCommand::execute(QVariant const &) {
            LOG_DEBUG << "#";
            m_Source.purgeUnavailableFiles();
        }

        void GenerateCompletionsCommand::execute(const QVariant &value) {
            QString prefix = Helpers::convertToString(value);
            LOG_DEBUG << prefix;
            m_Source.generateCompletions(prefix);
        }

        void SelectFilteredArtworksCommand::execute(const QVariant &value) {
            LOG_DEBUG << value;
            int selectionIndex = Helpers::convertToInt(value, -1);

            // selected index of combobox in the MainGrid.qml
            Models::FilteredArtworksListModel::SelectionType selectionType;
            switch (selectionIndex) {
                case 0: selectionType = Models::FilteredArtworksListModel::SelectAll; break;
                case 1: selectionType = Models::FilteredArtworksListModel::SelectNone; break;
                case 2: selectionType = Models::FilteredArtworksListModel::SelectModified; break;
                case 3: selectionType = Models::FilteredArtworksListModel::SelectImages; break;
                case 4: selectionType = Models::FilteredArtworksListModel::SelectVectors; break;
                case 5: selectionType = Models::FilteredArtworksListModel::SelectVideos; break;
                default: selectionType = Models::FilteredArtworksListModel::DontSelect; break;
            }

            m_Source.selectArtworksEx(selectionType);
        }

        void CheckWarningsCommand::execute(const QVariant &) {
            LOG_DEBUG << "#";
            m_Source.update();
        }

        void InitUploadHostCommand::execute(const QVariant &value) {
            LOG_DEBUG << value;
            bool masterPasswordCorrectOrEmpty = Helpers::convertToBool(value, false);
            m_Source.initializeAccounts(masterPasswordCorrectOrEmpty);
        }

        void UpdateLogsCommand::execute(const QVariant &value) {
            LOG_DEBUG << "#";
            bool moreLogs = Helpers::convertToBool(value, false);
            m_Target.updateLogs(moreLogs);
        }

        void InitUserDictionaryCommand::execute(const QVariant &) {
            LOG_DEBUG << "#";
            m_Target.initializeModel();
        }
    }
}

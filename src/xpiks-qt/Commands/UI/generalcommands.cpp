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
#include <Services/AutoComplete/autocompleteservice.h>

namespace Commands {
    namespace UI {
        QString convertToString(QVariant const &value, QString const &defaultValue = QString("")) {
            QString result = defaultValue;
            if (value.isValid()) {
                if (value.type() == QVariant::String) {
                    result = value.toString();
                }
            }
            return result;
        }

        int convertToInt(QVariant const &value, int defaultValue = 0) {
            int result = defaultValue;
            if (value.isValid()) {
                if (value.type() == QVariant::Int) {
                    result = value.toInt();
                }
            }
            return result;
        }

        void SetMasterPasswordCommand::execute(QVariant const &) {
            LOG_DEBUG << "#";
            m_Target.onMasterPasswordSet(m_Source);
        }

        void RemoveUnavailableFilesCommand::execute(QVariant const &) {
            LOG_DEBUG << "#";
            m_ArtworksListModel.purgeUnavailableFiles();
        }

        void GenerateCompletionsCommand::execute(const QVariant &value) {
            QString prefix = convertToString(value);
            LOG_DEBUG << prefix;
            m_AutoCompleteService.generateCompletions(prefix);
        }

        void SelectFilteredArtworksCommand::execute(const QVariant &value) {
            LOG_DEBUG << value;
            int selectionIndex = convertToInt(value, -1);

            // selected index of combobox in the MainGrid.qml
            Models::FilteredArtworksListModel::SelectionType selectionType;
            switch (selectionIndex) {
                case 0: selectionType = Models::FilteredArtworksListModel::SelectAll;
                case 1: selectionType = Models::FilteredArtworksListModel::SelectNone;
                case 2: selectionType = Models::FilteredArtworksListModel::SelectModified;
                case 3: selectionType = Models::FilteredArtworksListModel::SelectImages;
                case 4: selectionType = Models::FilteredArtworksListModel::SelectVectors;
                case 5: selectionType = Models::FilteredArtworksListModel::SelectVideos;
                default: selectionType = Models::FilteredArtworksListModel::DontSelect;
            }

            m_FilteredArtworksList.selectArtworksEx(selectionType);
        }
    }
}

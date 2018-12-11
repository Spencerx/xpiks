/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef UICOMMAND_H
#define UICOMMAND_H

#include <QObject>

namespace QMLExtensions {
    class UICommandID
    {
        Q_GADGET
    public:
        UICommandID() = delete;

        // enum values must start with capital letter
        enum CommandID {
            // ------ General ------
            SaveSession,
            SetMasterPassword,
            RemoveUnavailableFiles,
            GenerateCompletions,
            SelectFilteredArtworks,
            CheckWarnings,
            InitUploadHosts,
            UpdateLogs,
            InitUserDictionary,
            AddToUserDictionary,
            ActivateQuickBuffer,
            FixSpelling,
            SetupFindInArtworks,
            FindReplaceCandidates,
            ReplaceInFound,
            // ------ Selected -------
            SetupEditSelectedArtworks,
            EditSelectedArtworks,
            ReviewSpellingInSelected,
            ReviewDuplicatesInSelected,
            SetupExportMetadata,
            SetupWipeMetadata,
            RemoveSelected,
            SetupReimportMetadata,
            SetupCSVExportForSelected,
            SetupUpload,
            SetupCreatingArchives,
            SetupDeleteKeywordsInSelected,
            DeleteKeywordsInSelected,
            UpdateSelected,
            // ------- Single -------
            ReviewSpellingCombined,
            ReviewSpellingSingle,
            ReviewSpellingArtwork,
            ReviewDuplicatesSingle,
            ReviewDuplicatesCombined,
            ReviewDuplicatesArtwork,
            AcceptPresetCompletionForCombined,
            AcceptPresetCompletionForSingle,
            AcceptPresetCompletionForArtwork,
            InitSuggestionArtwork,
            InitSuggestionCombined,
            InitSuggestionSingle,
            GenerateCompletionsForArtwork,
            SetupProxyArtworkEdit,
            SetupArtworkEdit,
            ReviewArtworkInfo,
            CopyArtworkToQuickBuffer,
            CopyCombinedToQuickBuffer,
            FillArtworkFromQuickBuffer,
            PlainTextEdit,
            AppendSuggestedKeywords
        };
        Q_ENUM(CommandID)
    };
}

#endif // UICOMMAND_H

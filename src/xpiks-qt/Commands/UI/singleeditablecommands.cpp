/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "singleeditablecommands.h"
#include <Models/Editing/combinedartworksmodel.h>
#include <Models/Editing/artworkproxymodel.h>
#include <Models/Artworks/artworkslistmodel.h>
#include <Models/Artworks/filteredartworkslistmodel.h>
#include <Services/SpellCheck/spellchecksuggestionmodel.h>
#include <Services/SpellCheck/duplicatesreviewmodel.h>
#include <Services/SpellCheck/spellsuggestionstarget.h>
#include <Suggestion/keywordssuggestor.h>

namespace Commands {
    namespace UI {
        int convertToInt(QVariant const &value, int defaultValue = 0) {
            int result = defaultValue;
            if (value.isValid()) {
                if (value.type() == QVariant::Int) {
                    result = value.toInt();
                }
            }
            return result;
        }

        void FixSpellingInBasicModelCommand::execute(const QVariant &) {
            LOG_DEBUG << m_CommandID;
            auto &basicMetadataModel = dynamic_cast<Artworks::BasicMetadataModel&>(m_BasicModelSource.getBasicModel());
            m_SpellSuggestionsModel.setupModel(
                        std::make_shared<SpellCheck::BasicModelSuggestionTarget>(
                            basicMetadataModel, m_SpellCheckService),
                        Common::SpellCheckFlags::All);
        }

        void FixSpellingInArtworkProxyCommand::execute(QVariant const &) {
            LOG_DEBUG << "#";
            Artworks::ArtworksSnapshot snapshot({m_Source.getArtwork()});
            m_SpellSuggestionsModel.setupModel(
                        std::make_shared<SpellCheck::ArtworksSuggestionTarget>(
                            snapshot, m_SpellCheckService, m_ArtworksUpdater),
                        Common::SpellCheckFlags::All);
        }

        void FixSpellingForArtworkCommand::execute(QVariant const &value) {
            LOG_DEBUG << value;
            int index = convertToInt(value, -1);
            std::shared_ptr<Artworks::ArtworkMetadata> artwork;
            if (m_Source.tryGetArtwork(index, artwork)) {
                Artworks::ArtworksSnapshot snapshot({artwork});
                m_SpellSuggestionsModel.setupModel(
                            std::make_shared<SpellCheck::ArtworksSuggestionTarget>(
                                snapshot, m_SpellCheckService, m_ArtworksUpdater),
                            Common::SpellCheckFlags::All);
            } else {
                LOG_WARNING << "Cannot find artwork at" << index;
            }
        }

        void ShowDuplicatesForSingleCommand::execute(QVariant const &) {
            LOG_DEBUG << "#";
            m_Target.setupModel(m_Source.getArtwork()->getBasicMetadataModel());
        }

        void ShowDuplicatesForCombinedCommand::execute(QVariant const &) {
            LOG_DEBUG << "#";
            m_Target.setupModel(m_Source.getBasicModel());
        }

        void AcceptPresetCompletionForCombinedCommand::execute(QVariant const &value) {
            LOG_DEBUG << value;
            int completionID = convertToInt(value, 0);

            bool accepted = m_Target.acceptCompletionAsPreset(m_Source, completionID);
            LOG_INFO << "completion" << completionID << "accepted:" << accepted;
        }

        void AcceptPresetCompletionForSingleCommand::execute(QVariant const &value) {
            LOG_DEBUG << value;
            int completionID = convertToInt(value, 0);

            bool accepted = m_Target.acceptCompletionAsPreset(m_Source, completionID);
            LOG_INFO << "completion" << completionID << "accepted:" << accepted;
        }

        void InitSuggestionForArtworkCommand::execute(QVariant const &value) {
            LOG_DEBUG << value;
            int index = convertToInt(value, -1);
            std::shared_ptr<Artworks::ArtworkMetadata> artwork;
            if (m_Source.tryGetArtwork(index, artwork)) {
                m_Target.setExistingKeywords(artwork->getKeywords().toSet());
            }
        }

        void InitSuggestionForCombinedCommand::execute(QVariant const &) {
            LOG_DEBUG << "#";
            m_Target.setExistingKeywords(m_Source.getKeywords().toSet());
        }

        void InitSuggestionForSingleCommand::execute(QVariant const &) {
            LOG_DEBUG << "#";
            m_Target.setExistingKeywords(m_Source.getKeywords().toSet());
        }

    }
}

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
#include <Services/AutoComplete/autocompleteservice.h>
#include <Suggestion/keywordssuggestor.h>
#include <Helpers/uihelpers.h>

namespace Commands {
    namespace UI {
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
            int index = Helpers::convertToInt(value, -1);
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

        void ShowDuplicatesForArtworkCommand::execute(QVariant const &value) {
            LOG_DEBUG << value;
            int proxyIndex = Helpers::convertToInt(value, -1);
            std::shared_ptr<Artworks::ArtworkMetadata> artwork;
            if (m_Source.tryGetArtwork(proxyIndex, artwork)) {
                m_Target.setupModel(artwork->getBasicModel());
            }
        }

        void AcceptPresetCompletionForCombinedCommand::execute(QVariant const &value) {
            LOG_DEBUG << value;
            int completionID = Helpers::convertToInt(value, 0);

            bool accepted = m_Target.acceptCompletionAsPreset(m_Source, completionID);
            LOG_INFO << "completion" << completionID << "accepted:" << accepted;
        }

        void AcceptPresetCompletionForSingleCommand::execute(QVariant const &value) {
            LOG_DEBUG << value;
            int completionID = Helpers::convertToInt(value, 0);

            bool accepted = m_Target.acceptCompletionAsPreset(m_Source, completionID);
            LOG_INFO << "completion" << completionID << "accepted:" << accepted;
        }

        void AcceptPresetCompletionForArtworkCommand::execute(QVariant const &value) {
            LOG_DEBUG << value;
            int artworkIndex = 0;
            int completionID = 0;

            if (value.isValid()) {
                auto map = value.toMap();
                auto completionValue = map.value("completion", QVariant(0));
                if (completionValue.type() == QVariant::Int) {
                    completionID = completionValue.toInt();
                }
                auto indexValue = map.value("index", QVariant(0));
                if (indexValue.type() == QVariant::Int) {
                    artworkIndex = indexValue.toInt();
                }
            }

            m_Target.acceptCompletionAsPreset(artworkIndex, m_Source, completionID);
        }

        void GenerateCompletionsForArtworkCommand::execute(QVariant const &value) {
            LOG_DEBUG << value;
            int artworkIndex = 0;
            QString prefix;

            if (value.isValid()) {
                auto map = value.toMap();
                auto prefixValue = map.value("prefix", QVariant(""));
                if (prefixValue.type() == QVariant::String) {
                    prefix = prefixValue.toString();
                }
                auto indexValue = map.value("index", QVariant(0));
                if (indexValue.type() == QVariant::Int) {
                    artworkIndex = indexValue.toInt();
                }
            }

            std::shared_ptr<Artworks::ArtworkMetadata> artwork;
            if (m_Source.tryGetArtwork(artworkIndex, artwork)) {
                m_Target.generateCompletions(prefix, artwork->getBasicModel());
            }
        }

        void InitSuggestionForArtworkCommand::execute(QVariant const &value) {
            LOG_DEBUG << value;
            int index = Helpers::convertToInt(value, -1);
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

        void EditArtworkCommand::execute(const QVariant &value) {
            LOG_DEBUG << value;
            int proxyIndex = Helpers::convertToInt(value, -1);
            std::shared_ptr<Artworks::ArtworkMetadata> artwork;
            if (m_Source.tryGetArtwork(proxyIndex, artwork)) {
                m_Target.setSourceArtwork(artwork, proxyIndex);
            }
        }
    }
}

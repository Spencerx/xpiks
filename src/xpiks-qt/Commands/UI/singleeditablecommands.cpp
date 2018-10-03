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
#include <Commands/Base/icommandmanager.h>
#include <Commands/Editing/modifyartworkscommand.h>
#include <Commands/Editing/editartworkstemplate.h>
#include <Commands/artworksupdatetemplate.h>
#include <Commands/Base/compositecommandtemplate.h>
#include <Artworks/artworkssnapshot.h>
#include <Models/Editing/combinedartworksmodel.h>
#include <Models/Editing/artworkproxymodel.h>
#include <Models/Editing/quickbuffer.h>
#include <Models/Editing/icurrenteditable.h>
#include <Models/Artworks/artworkslistmodel.h>
#include <Models/Artworks/filteredartworkslistmodel.h>
#include <Services/iartworksupdater.h>
#include <Services/SpellCheck/spellchecksuggestionmodel.h>
#include <Services/SpellCheck/duplicatesreviewmodel.h>
#include <Services/SpellCheck/spellsuggestionstarget.h>
#include <Services/AutoComplete/autocompleteservice.h>
#include <Suggestion/keywordssuggestor.h>
#include <Helpers/uihelpers.h>

namespace Commands {
    namespace UI {
        void ReviewSpellingInBasicModelCommand::execute(const QVariant &) {
            LOG_DEBUG << m_CommandID;
            auto &basicMetadataModel = dynamic_cast<Artworks::BasicMetadataModel&>(m_BasicModelSource.getBasicModel());
            m_SpellSuggestionsModel.setupModel(
                        std::make_shared<SpellCheck::BasicModelSuggestionTarget>(
                            basicMetadataModel, m_SpellCheckService),
                        Common::SpellCheckFlags::All);
        }

        void ReviewSpellingInArtworkProxyCommand::execute(QVariant const &) {
            LOG_DEBUG << "#";
            Artworks::ArtworksSnapshot snapshot({m_Source.getArtwork()});
            m_SpellSuggestionsModel.setupModel(
                        std::make_shared<SpellCheck::ArtworksSuggestionTarget>(
                            snapshot, m_SpellCheckService, m_ArtworksUpdater),
                        Common::SpellCheckFlags::All);
        }

        void ReviewSpellingInArtworkCommand::execute(QVariant const &value) {
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

        void ReviewDuplicatesForSingleCommand::execute(QVariant const &) {
            LOG_DEBUG << "#";
            // do not use artwork here to eliminate infinite loop of
            // viewing duplicates and clicking "edit"
            m_Target.setupModel(m_Source.getArtwork()->getBasicMetadataModel());
        }

        void ReviewDuplicatesForCombinedCommand::execute(QVariant const &) {
            LOG_DEBUG << "#";
            m_Target.setupModel(m_Source.getBasicModel());
        }

        void ReviewDuplicatesForArtworkCommand::execute(QVariant const &value) {
            LOG_DEBUG << value;
            int proxyIndex = Helpers::convertToInt(value, -1);
            std::shared_ptr<Artworks::ArtworkMetadata> artwork;
            if (m_Source.tryGetArtwork(proxyIndex, artwork)) {
                m_Target.setupModel(Artworks::ArtworksSnapshot({artwork}));
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
            int proxyIndex = Helpers::convertToInt(value, -1);
            std::shared_ptr<Artworks::ArtworkMetadata> artwork;
            if (m_Source.tryGetArtwork(proxyIndex, artwork)) {                
                m_Source.registerCurrentItem(proxyIndex);
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

        void SetupArtworkEditCommand::execute(const QVariant &value) {
            LOG_DEBUG << value;
            int proxyIndex = Helpers::convertToInt(value, -1);
            std::shared_ptr<Artworks::ArtworkMetadata> artwork;
            if (m_Source.tryGetArtwork(proxyIndex, artwork)) {
                m_Target.setSourceArtwork(artwork, proxyIndex);
            }
        }

        void CopyArtworkToQuickBufferCommand::execute(const QVariant &value) {
            LOG_DEBUG << value;
            int proxyIndex = Helpers::convertToInt(value, -1);
            m_Source.copyToQuickBuffer(proxyIndex);
        }

        void CopyCombinedToQuickBufferCommand::execute(const QVariant &) {
            LOG_DEBUG << "#";
            m_Source.copyToQuickBuffer();
        }

        void FillArtworkFromQuickBufferCommand::execute(const QVariant &value) {
            int proxyIndex = Helpers::convertToInt(value, -1);
            std::shared_ptr<Artworks::ArtworkMetadata> artwork;
            if (m_FilteredArtworksList.tryGetArtwork(proxyIndex, artwork)) {
                Common::ArtworkEditFlags editFlags = Common::ArtworkEditFlags::None;

                QString title = m_QuickBuffer.getTitle();
                QString description = m_QuickBuffer.getDescription();
                QStringList keywords = m_QuickBuffer.getKeywords();

                if (!title.isEmpty()) { Common::SetFlag(editFlags, Common::ArtworkEditFlags::EditTitle); }
                if (!description.isEmpty()) { Common::SetFlag(editFlags, Common::ArtworkEditFlags::EditDescription); }
                if (!keywords.empty()) { Common::SetFlag(editFlags, Common::ArtworkEditFlags::EditKeywords); }

                using ArtworksTemplate = Commands::ICommandTemplate<Artworks::ArtworksSnapshot>;
                using ArtworksTemplateComposite = Commands::CompositeCommandTemplate<Artworks::ArtworksSnapshot>;

                m_CommandManager.processCommand(
                            std::make_shared<ModifyArtworksCommand>(
                                Artworks::ArtworksSnapshot({artwork}),
                                std::make_shared<ArtworksTemplateComposite>(
                                    std::initializer_list<std::shared_ptr<ArtworksTemplate>>{
                                        std::make_shared<EditArtworksTemplate>(
                                        title,
                                        description,
                                        keywords,
                                        editFlags),
                                        std::make_shared<Commands::ArtworksSnapshotUpdateTemplate>(
                                        m_Updater)})));
            }
        }
    }
}

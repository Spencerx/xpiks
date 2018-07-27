/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "currenteditableartwork.h"
#include <Artworks/artworkmetadata.h>
#include <Common/defines.h>
#include <Commands/commandmanager.h>
#include <KeywordsPresets/presetkeywordsmodel.h>
#include <Commands/Editing/expandpresettemplate.h>
#include <Artworks/artworkelement.h>
#include <Commands/Editing/deletekeywordstemplate.h>
#include <Commands/Base/icommand.h>
#include <Commands/Editing/modifyartworkscommand.h>
#include <Commands/Base/compositecommandtemplate.h>
#include <Commands/Base/templatedcommand.h>
#include <Commands/Editing/deletekeywordstemplate.h>
#include <Commands/Base/emptycommand.h>

namespace Models {
    using ArtworksTemplateComposite = Commands::CompositeCommandTemplate<Artworks::ArtworksSnapshot>;
    using ArtworksCommand = Commands::TemplatedCommand<Artworks::ArtworksSnapshot>;

    CurrentEditableArtwork::CurrentEditableArtwork(Artworks::ArtworkMetadata *artworkMetadata,
                                                   const std::shared_ptr<IArtworksCommandTemplate> &updateTemplate):
        m_ArtworkMetadata(artworkMetadata),
        m_UpdateTemplate(updateTemplate)
    {
        Q_ASSERT(artworkMetadata != nullptr);
        m_ArtworkMetadata->acquire();
    }

    CurrentEditableArtwork::~CurrentEditableArtwork() {
        if (m_ArtworkMetadata->release()) {
            LOG_WARNING << "Item #" << m_ArtworkMetadata->getItemID() << "could have been removed";
        }
    }

    size_t CurrentEditableArtwork::getOriginalIndex() const {
        return m_ArtworkMetadata->getLastKnownIndex();
    }

    Common::ID_t CurrentEditableArtwork::getItemID() {
        return m_ArtworkMetadata->getItemID();
    }

    QString CurrentEditableArtwork::getTitle() {
        return m_ArtworkMetadata->getTitle();
    }

    QString CurrentEditableArtwork::getDescription() {
        return m_ArtworkMetadata->getDescription();
    }

    QStringList CurrentEditableArtwork::getKeywords() {
        return m_ArtworkMetadata->getKeywords();
    }

    void CurrentEditableArtwork::setTitle(const QString &value) {
        m_ArtworkMetadata->setTitle(value);
    }

    void CurrentEditableArtwork::setDescription(const QString &value) {
        m_ArtworkMetadata->setDescription(value);
    }

    void CurrentEditableArtwork::setKeywords(const QStringList &keywords) {
        m_ArtworkMetadata->setKeywords(keywords);
    }

    bool CurrentEditableArtwork::hasKeywords(const QStringList &keywordsList) {
        return m_ArtworkMetadata->hasKeywords(keywordsList);
    }

    std::shared_ptr<Commands::ICommand> CurrentEditableArtwork::appendPreset(KeywordsPresets::ID_t presetID,
                                                                             KeywordsPresets::IPresetsManager &presetsManager) {
        using namespace Commands;
        auto command = std::make_shared<ModifyArtworksCommand>(
                        m_ArtworkMetadata,
                        std::make_shared<ArtworksTemplateComposite>({
                                                              std::make_shared<ExpandPresetTemplate>(presetsManager,
                                                              (KeywordsPresets::ID_t)presetID),
                                                              m_UpdateTemplate}));
        return command;
    }

    std::shared_ptr<Commands::ICommand> CurrentEditableArtwork::expandPreset(int keywordIndex,
                                                                             KeywordsPresets::ID_t presetID,
                                                                             KeywordsPresets::IPresetsManager &presetsManager) {
        using namespace Commands;
        auto command = std::make_shared<ModifyArtworksCommand>(
                        m_ArtworkMetadata,
                        std::make_shared<ArtworksTemplateComposite>({
                                                             std::make_shared<ExpandPresetTemplate>(presetsManager,
                                                             (KeywordsPresets::ID_t)presetID,
                                                             keywordIndex),
                                                             m_UpdateTemplate}));
        return command;

    }

    std::shared_ptr<Commands::ICommand> CurrentEditableArtwork::removePreset(KeywordsPresets::ID_t presetID,
                                                                             KeywordsPresets::IPresetsManager &presetsManager) {
        QStringList keywords;
        using namespace Commands;
        std::shared_ptr<ICommand> command;
        if (presetsManager.tryGetPreset(presetID, keywords)) {
            for (auto &keyword: keywords) {
                keyword = keyword.toLower();
            }

            command = std::make_shared<ModifyArtworksCommand>(
                          m_ArtworkMetadata,
                          std::make_shared<ArtworksTemplateComposite>({
                                                                std::make_shared<DeleteKeywordsTemplate>(
                                                                          keywords.toSet(), false),
                                                                          m_UpdateTemplate}));
        } else {
            command = std::make_shared<EmptyCommand>();
        }

        return command;
    }

    std::shared_ptr<Commands::ICommand> CurrentEditableArtwork::inspect() {
        return std::make_shared<ArtworksCommand>(
                        Artworks::ArtworksSnapshot({m_ArtworkMetadata}),
                        m_InspectTemplate);
    }

    std::shared_ptr<Commands::ICommand> CurrentEditableArtwork::update() {
        return std::make_shared<ArtworksCommand>(
                        Artworks::ArtworksSnapshot({m_ArtworkMetadata}),
                        m_UpdateTemplate);
    }

    std::shared_ptr<Commands::ICommand> CurrentEditableArtwork::applyEdits(const QString &title,
                                            const QString &description,
                                            const QStringList &keywords) {
        Common::ArtworkEditFlags flags = Common::ArtworkEditFlags::None;
        if (!title.isEmpty()) { Common::SetFlag(flags, Common::ArtworkEditFlags::EditTitle); }
        if (!description.isEmpty()) { Common::SetFlag(flags, Common::ArtworkEditFlags::EditDescription); }
        if (!keywords.empty()) { Common::SetFlag(flags, Common::ArtworkEditFlags::EditKeywords); }

        using namespace Commands;
        auto command = std::make_shared<ModifyArtworksCommand>(
                    m_ArtworkMetadata,
                    std::make_shared<CompositeCommandTemplate>(
                        std::initializer_list<std::shared_ptr<ArtworksTemplate>>{
                            std::make_shared<EditArtworksTemplate>(
                            title, description, keywords, flags),
                            m_UpdateTemplate
                        }));
        return command;
    }
}

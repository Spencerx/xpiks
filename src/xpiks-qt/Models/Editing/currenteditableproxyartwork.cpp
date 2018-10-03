/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "currenteditableproxyartwork.h"
#include <Models/Editing/artworkproxybase.h>
#include <Commands/Base/callbackcommand.h>
#include <Commands/Base/emptycommand.h>

namespace Models {
    CurrentEditableProxyArtwork::CurrentEditableProxyArtwork(Models::ArtworkProxyBase &artworkProxy):
        m_ArtworkProxy(artworkProxy)
    {
    }

    Common::ID_t CurrentEditableProxyArtwork::getItemID() {
        return m_ArtworkProxy.getSpecialItemID();
    }

    QString CurrentEditableProxyArtwork::getTitle() {
        return m_ArtworkProxy.getTitle();
    }

    QString CurrentEditableProxyArtwork::getDescription() {
        return m_ArtworkProxy.getDescription();
    }

    QStringList CurrentEditableProxyArtwork::getKeywords() {
        return m_ArtworkProxy.getKeywords();
    }

    bool CurrentEditableProxyArtwork::hasKeywords(const QStringList &keywordsList) {
        return m_ArtworkProxy.hasKeywords(keywordsList);
    }

    std::shared_ptr<Commands::ICommand> CurrentEditableProxyArtwork::setTitle(const QString &title) {
        return std::make_shared<Commands::CallbackCommand>([this, title]() {
            this->m_ArtworkProxy.setTitle(title);
        });
    }

    std::shared_ptr<Commands::ICommand> CurrentEditableProxyArtwork::setDescription(const QString &description) {
        return std::make_shared<Commands::CallbackCommand>([this, description]() {
            this->m_ArtworkProxy.setDescription(description);
        });
    }

    std::shared_ptr<Commands::ICommand> CurrentEditableProxyArtwork::setKeywords(const QStringList &keywords) {
        return std::make_shared<Commands::CallbackCommand>([this, keywords]() {
            this->m_ArtworkProxy.setKeywords(keywords);
        });
    }

    std::shared_ptr<Commands::ICommand> CurrentEditableProxyArtwork::appendKeywords(const QStringList &keywords) {
        return std::make_shared<Commands::CallbackCommand>([this, keywords]() {
            this->m_ArtworkProxy.doAppendKeywords(keywords);
        });
    }

    std::shared_ptr<Commands::ICommand> CurrentEditableProxyArtwork::appendPreset(KeywordsPresets::ID_t presetID,
                                                                                  KeywordsPresets::IPresetsManager &presetsManager) {
        return std::make_shared<Commands::CallbackCommand>([this, presetID, &presetsManager]() {
            this->m_ArtworkProxy.doAppendPreset(presetID, presetsManager);
        });
    }

    std::shared_ptr<Commands::ICommand> CurrentEditableProxyArtwork::expandPreset(int keywordIndex,
                                                                                  KeywordsPresets::ID_t presetID,
                                                                                  KeywordsPresets::IPresetsManager &presetsManager) {
        return std::make_shared<Commands::CallbackCommand>([this, keywordIndex, presetID, &presetsManager]() {
            this->m_ArtworkProxy.doExpandPreset(keywordIndex, presetID, presetsManager);
        });
    }

    std::shared_ptr<Commands::ICommand> CurrentEditableProxyArtwork::removePreset(KeywordsPresets::ID_t presetID,
                                                                                  KeywordsPresets::IPresetsManager &presetsManager) {
        return std::make_shared<Commands::CallbackCommand>([this, presetID, &presetsManager]() {
            this->m_ArtworkProxy.doRemovePreset(presetID, presetsManager);
        });
    }

    std::shared_ptr<Commands::ICommand> CurrentEditableProxyArtwork::update() {
        return std::make_shared<Commands::EmptyCommand>();
    }

    std::shared_ptr<Commands::ICommand> CurrentEditableProxyArtwork::applyEdits(const QString &title,
                                                                                const QString &description,
                                                                                const QStringList &keywords) {

        return std::make_shared<Commands::CallbackCommand>([this, title, description, keywords](){
            if (!title.isEmpty()) {
                this->m_ArtworkProxy.setTitle(title);
            }

            if (!description.isEmpty()) {
                this->m_ArtworkProxy.setDescription(description);
            }

            if (!keywords.isEmpty()) {
                this->m_ArtworkProxy.setKeywords(keywords);
            }

            // TODO: spellcheck me
        });
    }
}

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
#include <Commands/Base/simplecommand.h>
#include <Commands/Base/emptycommand.h>

namespace Models {
    CurrentEditableProxyArtwork::CurrentEditableProxyArtwork(Models::ArtworkProxyBase &artworkProxy):
        m_ArtworkProxy(artworkProxy)
    {
    }

    qint64 CurrentEditableProxyArtwork::getItemID() {
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

    void CurrentEditableProxyArtwork::setTitle(const QString &title) {
        m_ArtworkProxy.setTitle(title);
    }

    void CurrentEditableProxyArtwork::setDescription(const QString &description) {
        m_ArtworkProxy.setDescription(description);
    }

    void CurrentEditableProxyArtwork::setKeywords(const QStringList &keywords) {
        m_ArtworkProxy.setKeywords(keywords);
    }

    bool CurrentEditableProxyArtwork::hasKeywords(const QStringList &keywordsList) {
        return m_ArtworkProxy.hasKeywords(keywordsList);
    }

    std::shared_ptr<Commands::ICommand> CurrentEditableProxyArtwork::appendPreset(KeywordsPresets::ID_t presetID,
                                                                                  KeywordsPresets::IPresetsManager &presetsManager) {
        return std::shared_ptr<Commands::ICommand>(
                    new Commands::SimpleCommand([this, presetID, &presetsManager]() {
            this->m_ArtworkProxy.doAppendPreset(presetID, presetsManager);
        }));
    }

    std::shared_ptr<Commands::ICommand> CurrentEditableProxyArtwork::expandPreset(int keywordIndex,
                                                                                  KeywordsPresets::ID_t presetID,
                                                                                  KeywordsPresets::IPresetsManager &presetsManager) {
        return std::shared_ptr<Commands::ICommand>(
                    new Commands::SimpleCommand([this, keywordIndex, presetID, &presetsManager]() {
            this->m_ArtworkProxy.doExpandPreset(keywordIndex, presetID, presetsManager);
        }));
    }

    std::shared_ptr<Commands::ICommand> CurrentEditableProxyArtwork::removePreset(KeywordsPresets::ID_t presetID,
                                                                                  KeywordsPresets::IPresetsManager &presetsManager) {
        return std::shared_ptr<Commands::ICommand>(
                    new Commands::SimpleCommand([this, presetID, &presetsManager]() {
            this->m_ArtworkProxy.doRemovePreset(presetID, presetsManager);
        }));
    }

    std::shared_ptr<Commands::ICommand> CurrentEditableProxyArtwork::inspect() {
        // TODO:
    }

    std::shared_ptr<Commands::ICommand> CurrentEditableProxyArtwork::update() {
        return std::shared_ptr<Commands::ICommand>(new Commands::EmptyCommand());
    }
}

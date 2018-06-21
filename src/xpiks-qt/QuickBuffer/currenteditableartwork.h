/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef CURRENTITEMARTWORK_H
#define CURRENTITEMARTWORK_H

#include "icurrenteditable.h"

namespace Artworks {
    class ArtworkMetadata;
}

namespace Commands {
    class ICommandManager;
}

namespace QuickBuffer {
    class CurrentEditableArtwork : public ICurrentEditable
    {
    public:
        CurrentEditableArtwork(Artworks::ArtworkMetadata *artworkMetadata,
                               Commands::ICommandManager &commandManager);
        virtual ~CurrentEditableArtwork();

    public:
        size_t getOriginalIndex() const;

        // ICurrentEditable interface
    public:
        virtual Common::ID_t getItemID() override;

        virtual QString getTitle() override;
        virtual QString getDescription() override;
        virtual QStringList getKeywords() override;

        virtual void setTitle(const QString &value) override;
        virtual void setDescription(const QString &value) override;
        virtual void setKeywords(const QStringList &keywords) override;

        virtual bool appendPreset(KeywordsPresets::ID_t presetID) override;
        virtual bool expandPreset(int keywordIndex, KeywordsPresets::ID_t presetID) override;
        virtual bool removePreset(KeywordsPresets::ID_t presetID) override;

        virtual bool hasKeywords(const QStringList &keywordsList) override;

        virtual void spellCheck() override;
        virtual void update() override;

    private:
        Commands::ICommandManager * const m_CommandManager;
        Artworks::ArtworkMetadata *m_ArtworkMetadata;
    };
}

#endif // CURRENTITEMARTWORK_H

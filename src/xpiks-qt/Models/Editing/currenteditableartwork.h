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
#include <Commands/Base/icommandtemplate.h>
#include <Artworks/artworkssnapshot.h>

namespace Artworks {
    class ArtworkMetadata;
}

namespace Models {
    class CurrentEditableArtwork : public ICurrentEditable
    {
        using IArtworksCommandTemplate = Commands::ICommandTemplate<Artworks::ArtworksSnapshot>;

    public:
        CurrentEditableArtwork(Artworks::ArtworkMetadata *artworkMetadata,
                               const std::shared_ptr<IArtworksCommandTemplate> &inspectTemplate,
                               const std::shared_ptr<IArtworksCommandTemplate> &updateTemplate);
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

        virtual bool hasKeywords(const QStringList &keywordsList) override;

        virtual std::shared_ptr<Commands::ICommand> appendPreset(KeywordsPresets::ID_t presetID,
                                                                 KeywordsPresets::IPresetsManager &presetsManager) override;
        virtual std::shared_ptr<Commands::ICommand> expandPreset(int keywordIndex,
                                                                 KeywordsPresets::ID_t presetID,
                                                                 KeywordsPresets::IPresetsManager &presetsManager) override;
        virtual std::shared_ptr<Commands::ICommand> removePreset(KeywordsPresets::ID_t presetID,
                                                                 KeywordsPresets::IPresetsManager &presetsManager) override;

        virtual std::shared_ptr<Commands::ICommand> inspect() override;
        virtual std::shared_ptr<Commands::ICommand> update() override;

    private:
        Artworks::ArtworkMetadata *m_ArtworkMetadata;
        std::shared_ptr<IArtworksCommandTemplate> m_InspectTemplate;
        std::shared_ptr<IArtworksCommandTemplate> m_UpdateTemplate;
    };
}

#endif // CURRENTITEMARTWORK_H

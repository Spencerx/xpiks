/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef CURRENTEDITABLEPROXYARTWORK_H
#define CURRENTEDITABLEPROXYARTWORK_H

#include <memory>

#include <QString>
#include <QStringList>

#include "Common/types.h"
#include "KeywordsPresets/presetmodel.h"
#include "Models/Editing/icurrenteditable.h"

namespace Commands {
    class ICommand;
}

namespace KeywordsPresets {
    class IPresetsManager;
}

namespace Models {
    class ArtworkProxyBase;
}

namespace Models {
    class CurrentEditableProxyArtwork : public ICurrentEditable
    {
    public:
        CurrentEditableProxyArtwork(Models::ArtworkProxyBase &artworkProxy);

        // ICurrentEditable interface
    public:
        virtual Common::ID_t getItemID() override;
        virtual QString getTitle() override;
        virtual QString getDescription() override;
        virtual QStringList getKeywords() override;
        virtual bool hasKeywords(const QStringList &keywordsList) override;

        virtual std::shared_ptr<Commands::ICommand> setTitle(const QString &title) override;
        virtual std::shared_ptr<Commands::ICommand> setDescription(const QString &description) override;
        virtual std::shared_ptr<Commands::ICommand> setKeywords(const QStringList &keywords) override;
        virtual std::shared_ptr<Commands::ICommand> appendKeywords(const QStringList &keywords) override;

        virtual std::shared_ptr<Commands::ICommand> appendPreset(KeywordsPresets::ID_t presetID,
                                                                 KeywordsPresets::IPresetsManager &presetsManager) override;
        virtual std::shared_ptr<Commands::ICommand> expandPreset(int keywordIndex,
                                                                 KeywordsPresets::ID_t presetID,
                                                                 KeywordsPresets::IPresetsManager &presetsManager) override;
        virtual std::shared_ptr<Commands::ICommand> removePreset(KeywordsPresets::ID_t presetID,
                                                                 KeywordsPresets::IPresetsManager &presetsManager) override;

        virtual std::shared_ptr<Commands::ICommand> update() override;        

        virtual std::shared_ptr<Commands::ICommand> applyEdits(const QString &title,
                                                               const QString &description,
                                                               const QStringList &keywords) override;

    private:
        Models::ArtworkProxyBase &m_ArtworkProxy;
    };
}

#endif // CURRENTEDITABLEPROXYARTWORK_H

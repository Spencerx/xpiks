/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef ICURRENTEDITABLE_H
#define ICURRENTEDITABLE_H

#include <memory>

#include <QString>
#include <QStringList>

#include "Common/types.h"
#include "KeywordsPresets/presetmodel.h"

namespace KeywordsPresets {
    class IPresetsManager;
}

namespace Commands {
    class ICommand;
}

namespace Models {
    class ICurrentEditable {
    public:
        virtual ~ICurrentEditable() { }

        virtual Common::ID_t getItemID() = 0;
        virtual QString getTitle() = 0;
        virtual QString getDescription() = 0;
        virtual QStringList getKeywords() = 0;
        virtual bool hasKeywords(const QStringList &keywordsList) = 0;

        virtual std::shared_ptr<Commands::ICommand> setTitle(const QString &title) = 0;
        virtual std::shared_ptr<Commands::ICommand> setDescription(const QString &description) = 0;
        virtual std::shared_ptr<Commands::ICommand> setKeywords(const QStringList &keywords) = 0;
        virtual std::shared_ptr<Commands::ICommand> appendKeywords(const QStringList &keywords) = 0;

        virtual std::shared_ptr<Commands::ICommand> appendPreset(KeywordsPresets::ID_t presetID,
                                                                 KeywordsPresets::IPresetsManager &presetsManager) = 0;
        virtual std::shared_ptr<Commands::ICommand> expandPreset(int keywordIndex,
                                                                 KeywordsPresets::ID_t presetID,
                                                                 KeywordsPresets::IPresetsManager &presetsManager) = 0;
        virtual std::shared_ptr<Commands::ICommand> removePreset(KeywordsPresets::ID_t presetID,
                                                                 KeywordsPresets::IPresetsManager &presetsManager) = 0;

        virtual std::shared_ptr<Commands::ICommand> update() = 0;

        virtual std::shared_ptr<Commands::ICommand> applyEdits(const QString &title,
                                                               const QString &description,
                                                               const QStringList &keywords) = 0;
    };

    class ICurrentEditableSource {
    public:
        virtual ~ICurrentEditableSource() {}
        virtual std::shared_ptr<ICurrentEditable> getCurrentEditable() const = 0;
    };
}

#endif // ICURRENTEDITABLE_H

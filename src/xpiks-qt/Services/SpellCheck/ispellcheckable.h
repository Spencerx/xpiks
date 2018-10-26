/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef ISPELLCHECKABLE_H
#define ISPELLCHECKABLE_H

#include <cstddef>
#include <memory>
#include <vector>

#include <QString>
#include <QStringList>

#include "Common/flags.h"

namespace Artworks {
    struct KeywordItem;
}

namespace SpellCheck {
    class KeywordSpellSuggestions;

    class ISpellCheckable {
    public:
        virtual ~ISpellCheckable() {}
        virtual Common::KeywordReplaceResult fixKeywordSpelling(size_t index, const QString &existing, const QString &replacement) = 0;
        virtual bool fixDescriptionSpelling(const QString &word, const QString &replacement) = 0;
        virtual bool fixTitleSpelling(const QString &word, const QString &replacement) = 0;
        virtual std::vector<Artworks::KeywordItem> retrieveMisspelledKeywords() = 0;
        virtual QStringList retrieveMisspelledTitleWords() = 0;
        virtual QStringList retrieveMisspelledDescriptionWords() = 0;
        virtual bool processFailedKeywordReplacements(const std::vector<std::shared_ptr<SpellCheck::KeywordSpellSuggestions> > &candidatesForRemoval) = 0;
        virtual void afterReplaceCallback() = 0;
    };
}

#endif // ISPELLCHECKABLE_H

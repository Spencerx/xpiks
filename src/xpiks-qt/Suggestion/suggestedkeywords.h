/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SUGGESTEDKEYWORDS_H
#define SUGGESTEDKEYWORDS_H

#include <QHash>
#include <QSet>
#include <QString>
#include <QStringList>

namespace Suggestion {
    class SuggestedKeywords
    {
    public:
        SuggestedKeywords();

    public:
        QStringList const &getSuggestedKeywords() const { return m_SuggestedKeywords; }
        QStringList const &getOtherKeywords() const { return m_OtherKeywords; }

    public:
        void addKeywords(QSet<QString> const &keywords);
        void removeKeywords(QSet<QString> const &keywords);
        void setExistingKeywords(QSet<QString> const &existingKeywords);
        void reset();
        void updateSuggestion(int artworksCount);

    private:
        void doUpdateSuggestion(int artworksCount);
        void accountKeywords(QSet<QString> const &keywords, int sign);

    private:
        QHash<QString, int> m_KeywordsHash;
        QSet<QString> m_ExistingKeywords;
        QStringList m_SuggestedKeywords;
        QStringList m_OtherKeywords;
    };
}

#endif // SUGGESTEDKEYWORDS_H

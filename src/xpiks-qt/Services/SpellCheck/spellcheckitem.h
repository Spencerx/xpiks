/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SPELLCHECKITEM_H
#define SPELLCHECKITEM_H

#include <vector>
#include <memory>
#include <QStringList>
#include <QObject>
#include <QHash>
#include <functional>
#include <Common/flags.h>
#include <Common/wordanalysisresult.h>

namespace Artworks {
    class ArtworkMetadata;
    class BasicKeywordsModel;
}

namespace SpellCheck {
    class SpellCheckQueryItem
    {
    public:
        SpellCheckQueryItem(size_t index, const QString &word):
            m_Word(word),
            m_Index(index),
            m_IsCorrect(true),
            m_IsDuplicate(false)
        { }

        SpellCheckQueryItem(const SpellCheckQueryItem &copy):
            m_Word(copy.m_Word),
            m_Index(copy.m_Index),
            m_Suggestions(copy.m_Suggestions),
            m_Stem(copy.m_Stem),
            m_IsCorrect(copy.m_IsCorrect),
            m_IsDuplicate(copy.m_IsDuplicate)
        { }

        QString m_Word;
        size_t m_Index;
        QStringList m_Suggestions;
        QString m_Stem;
        volatile bool m_IsCorrect;
        volatile bool m_IsDuplicate;
    };

    class SpellCheckItem: public QObject
    {
        Q_OBJECT
    public:
        /*
         * This class contains BasicKeywordsModel instead of ArtworkMetadata because
         * spelling is also checked in classes that are not Artworks like
         * Combined model, QuickBuffer etc.
         */
        SpellCheckItem(Artworks::BasicKeywordsModel *spellCheckable,
                       Common::SpellCheckFlags spellCheckFlags,
                       Common::WordAnalysisFlags wordAnalysisFlags);
        // this constructor used for trick with user dictionary update
        SpellCheckItem(Artworks::BasicKeywordsModel *spellCheckable,
                       const QStringList &keywordsToCheck,
                       Common::WordAnalysisFlags wordAnalysisFlags);
        virtual ~SpellCheckItem();

    private:
        void addWords(const QStringList &words, int startingIndex, const std::function<bool (const QString &word)> &pred);

    public:
        void submitSpellCheckResult();
        void accountResults();
        void requestSuggestions() { m_NeedsSuggestions = true; }

    public:
        bool getIsOnlyOneKeyword() const { return m_OnlyOneKeyword; }
        bool needsSuggestions() const { return m_NeedsSuggestions; }
        const std::vector<std::shared_ptr<SpellCheckQueryItem> > &getQueries() const { return m_QueryItems; }
        Common::WordAnalysisFlags getWordAnalysisFlags() const { return m_WordAnalysisFlags; }
        const QHash<QString, Common::WordAnalysisResult> &getHash() const { return m_SpellCheckResults; }

    signals:
        void resultsReady(Common::SpellCheckFlags flags, int index);

    private:
        Artworks::BasicKeywordsModel *m_BasicModel;
        std::vector<std::shared_ptr<SpellCheckQueryItem> > m_QueryItems;
        QHash<QString, Common::WordAnalysisResult> m_SpellCheckResults;
        Common::WordAnalysisFlags m_WordAnalysisFlags;
        Common::SpellCheckFlags m_SpellCheckFlags;
        volatile bool m_NeedsSuggestions;
        volatile bool m_OnlyOneKeyword;
    };

    class ArtworkSpellCheckItem: public SpellCheckItem {
        Q_OBJECT
    public:
        ArtworkSpellCheckItem(Artworks::ArtworkMetadata *artwork,
                              Common::SpellCheckFlags spellCheckFlags,
                              Common::WordAnalysisFlags wordAnalysisFlags);
        ArtworkSpellCheckItem(Artworks::ArtworkMetadata *artwork,
                              const QStringList &keywordsToCheck,
                              Common::WordAnalysisFlags wordAnalysisFlags);

    public:
        Artworks::ArtworkMetadata *getArtwork() const { return m_Artwork; }

    private:
        Artworks::ArtworkMetadata *m_Artwork;
    };
}

#endif // SPELLCHECKITEM_H

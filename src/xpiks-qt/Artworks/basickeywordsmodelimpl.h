/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef BASICKEYWORDSMODELIMPL_H
#define BASICKEYWORDSMODELIMPL_H

#include <QStringList>
#include <QVariant>
#include <QByteArray>
#include <QHash>
#include <QSet>
#include <QVector>
#include <QHash>
#include <vector>
#include <memory>
#include "keyword.h"
#include "../Common/flags.h"
#include "iartworkmetadata.h"
#include "../Common/wordanalysisresult.h"

namespace SpellCheck {
    class SpellCheckQueryItem;
    class KeywordSpellSuggestions;
    class SpellCheckItem;
    class SpellCheckInfo;
}

namespace Artworks {
    class BasicKeywordsModel;

    class BasicKeywordsModelImpl
    {
        friend class BasicKeywordsModel;

    public:
        BasicKeywordsModelImpl();
        virtual ~BasicKeywordsModelImpl() { }

    public:
        size_t getKeywordsSize() { return m_KeywordsList.size(); }
        QSet<QString> getKeywordsSet() const { return m_KeywordsSet; }
        bool areKeywordsEmpty() const { return m_KeywordsList.empty(); }
        virtual QString getKeywordsString();

    public:
        inline Keyword &accessKeyword(size_t row) { Q_ASSERT(row < m_KeywordsList.size()); return m_KeywordsList.at(row); }

    public:
        bool appendKeyword(const QString &keyword, bool dryRun=false);
        void takeKeywordAt(size_t index, QString &removedKeyword, bool &wasCorrect);
        bool prepareAppend(const QStringList &keywordsList, size_t &addedCount);
        size_t appendKeywords(const QStringList &keywordsList, bool dryRun=false);
        bool canEditKeyword(size_t index, const QString &replacement) const;
        bool editKeyword(size_t index, const QString &replacement);
        bool replaceKeyword(size_t index, const QString &existing, const QString &replacement);
        bool moveKeyword(size_t from, size_t to);
        bool clearKeywords();
        bool containsKeyword(const QString &searchTerm, Common::SearchFlags searchFlags=Common::SearchFlags::Keywords);
        bool hasKeywordsSpellError() const;
        bool hasKeywordsDuplicates() const;
        bool findKeywordsIndices(const QSet<QString> &keywordsToFind, bool caseSensitive, std::vector<int> &foundIndices);
        bool hasKeywords(const QStringList &keywordsList) const;
        QStringList generateStringList();

    public:
        std::vector<KeywordItem> retrieveMisspelledKeywords();
        std::vector<KeywordItem> retrieveDuplicatedKeywords();
        Common::KeywordReplaceResult fixKeywordSpelling(size_t index, const QString &existing, const QString &replacement);
        bool processFailedKeywordReplacements(const std::vector<std::shared_ptr<SpellCheck::KeywordSpellSuggestions> > &candidatesForRemoval,
                                              std::vector<int> &indicesToRemove);

    public:
        bool replaceInKeywords(const QString &replaceWhat, const QString &replaceTo,
                               Common::SearchFlags flags,
                               QVector<int> &indicesToRemove,
                               QVector<int> &indicesToUpdate);

    public:
        bool containsKeywords(const QStringList &keywordsList);

    public:
        void resetSpellCheckResults();
        void resetDuplicatesInfo();
        bool canBeAdded(const QString &keyword) const;

    public:
        bool hasKeyword(const QString &keyword);

    public:
        void setSpellCheckResults(const std::vector<std::shared_ptr<SpellCheck::SpellCheckQueryItem> > &items);
        bool isReplacedADuplicate(size_t index, const QString &existingPrev,
                                        const QString &replacement) const;

    private:
        std::vector<Keyword> m_KeywordsList;
        QSet<QString> m_KeywordsSet;
    };
}

#endif // BASICKEYWORDSMODELIMPL_H

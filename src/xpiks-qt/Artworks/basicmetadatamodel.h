/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef BASICMETADATAMODEL_H
#define BASICMETADATAMODEL_H

#include <cstddef>
#include <memory>
#include <vector>

#include <QHash>
#include <QMetaType>
#include <QObject>
#include <QReadWriteLock>
#include <QSet>
#include <QString>
#include <QStringList>

#include "Artworks/basickeywordsmodel.h"
#include "Artworks/iartworkmetadata.h"
#include "Common/flags.h"
#include "Services/SpellCheck/ispellcheckable.h"

namespace Common {
    struct WordAnalysisResult;
}

namespace SpellCheck {
    class KeywordSpellSuggestions;
    class SpellCheckInfo;
    class SpellCheckQueryItem;
}

namespace Artworks {
    struct KeywordItem;

    class BasicMetadataModel :
            public BasicKeywordsModel,
            public IArtworkMetadata,
            public SpellCheck::ISpellCheckable
    {
        Q_OBJECT
        Q_PROPERTY(bool hasTitleSpellErrors READ hasTitleSpellError NOTIFY titleSpellingChanged)
        Q_PROPERTY(bool hasDescriptionSpellErrors READ hasDescriptionSpellError NOTIFY descriptionSpellingChanged)
    public:
        BasicMetadataModel(SpellCheck::SpellCheckInfo &spellCheckInfo, QObject *parent=nullptr);

    public:
        Q_INVOKABLE bool hasAnySpellingErrors() { return hasSpellErrors(); }

    public:
        SpellCheck::SpellCheckInfo &getSpellCheckInfo() const { return m_SpellCheckInfo; }
        QString getDescription();
        QString getTitle();

    public:
#ifdef CORE_TESTS
        void initialize(const QString &title, const QString &description, const QString &rawKeywords);
#endif

    public:
        virtual void setSpellCheckResults(const QHash<QString, Common::WordAnalysisResult> &results, Common::SpellCheckFlags flags);
        virtual QStringList retrieveMisspelledDescriptionWords() override;
        virtual QStringList retrieveMisspelledTitleWords() override;
        virtual bool fixDescriptionSpelling(const QString &word, const QString &replacement) override;
        virtual bool fixTitleSpelling(const QString &word, const QString &replacement) override;
        virtual void setKeywordsSpellCheckResults(const std::vector<std::shared_ptr<SpellCheck::SpellCheckQueryItem> > &items) override;
        virtual bool processFailedKeywordReplacements(const std::vector<std::shared_ptr<SpellCheck::KeywordSpellSuggestions> > &candidatesForRemoval) override;
        virtual std::vector<KeywordItem> retrieveMisspelledKeywords() override;
        virtual Common::KeywordReplaceResult fixKeywordSpelling(size_t index, const QString &existing, const QString &replacement) override;
        virtual void afterReplaceCallback() override;
        virtual QStringList getDescriptionWords();
        virtual QStringList getTitleWords();
        virtual bool expandPreset(size_t keywordIndex, const QStringList &presetList) override;
        virtual bool appendPreset(const QStringList &presetList) override;

    private:
        bool replaceInDescription(const QString &replaceWhat, const QString &replaceTo,
                                  Common::SearchFlags flags);
        bool replaceInTitle(const QString &replaceWhat, const QString &replaceTo,
                            Common::SearchFlags flags);

    public:
        virtual bool replace(const QString &replaceWhat, const QString &replaceTo, Common::SearchFlags flags) override;
        bool hasDescriptionSpellError();
        bool hasTitleSpellError();
        bool hasDescriptionWordSpellError(const QString &word);
        bool hasTitleWordSpellError(const QString &word);
        virtual bool hasSpellErrors() override;
        virtual bool hasDuplicates() override;

    public:
        // IArtworkMetadata
        // c++ is still not capable of picking abstract implementations from Base class
        virtual bool editKeyword(size_t index, const QString &replacement) override { return BasicKeywordsModel::editKeyword(index, replacement); }
        virtual bool removeKeywordAt(size_t index, QString &removedKeyword) override { return BasicKeywordsModel::removeKeywordAt(index, removedKeyword); }
        virtual bool removeLastKeyword(QString &removedKeyword) override { return BasicKeywordsModel::removeLastKeyword(removedKeyword); }
        virtual bool appendKeyword(const QString &keyword) override { return BasicKeywordsModel::appendKeyword(keyword); }
        virtual size_t appendKeywords(const QStringList &keywordsList) override { return BasicKeywordsModel::appendKeywords(keywordsList); }
        virtual bool clearKeywords() override { return BasicKeywordsModel::clearKeywords(); }
        virtual QString getKeywordsString() override { return BasicKeywordsModel::getKeywordsString(); }
        virtual void setKeywords(const QStringList &keywords) override { return BasicKeywordsModel::setKeywords(keywords); }
        virtual bool removeKeywords(const QSet<QString> &keywords, bool caseSensitive) override { return BasicKeywordsModel::removeKeywords(keywords, caseSensitive); }
        virtual bool hasKeywords(const QStringList &keywordsList) override { return BasicKeywordsModel::hasKeywords(keywordsList); }
        virtual bool moveKeyword(size_t from, size_t to) override { return BasicKeywordsModel::moveKeyword(from, to); }

    public:
        virtual bool setDescription(const QString &value) override;
        virtual bool setTitle(const QString &value) override;
        virtual bool isEmpty() override;
        bool isTitleEmpty();
        bool isDescriptionEmpty();
        void clearModel();

    public:
        virtual void notifySpellCheckResults(Common::SpellCheckFlags flags) override;
        void notifyDescriptionSpellingChanged();
        void notifyTitleSpellingChanged();

    signals:
        void titleSpellingChanged();
        void descriptionSpellingChanged();

    private:
        void updateDescriptionSpellErrors(const QHash<QString, Common::WordAnalysisResult> &results, bool withStemInfo);
        void updateTitleSpellErrors(const QHash<QString, Common::WordAnalysisResult> &results, bool withStemInfo);

    private:
        QReadWriteLock m_DescriptionLock;
        QReadWriteLock m_TitleLock;
        SpellCheck::SpellCheckInfo &m_SpellCheckInfo;
        QString m_Description;
        QString m_Title;
    };
}

Q_DECLARE_METATYPE(Artworks::BasicMetadataModel *)

#endif // BASICMETADATAMODEL_H

/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef KEYWORDSPELLSUGGESTIONS_H
#define KEYWORDSPELLSUGGESTIONS_H

#include <cstddef>
#include <memory>
#include <vector>

#include <QAbstractListModel>
#include <QHash>
#include <QMetaType>
#include <QModelIndex>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <Qt>

#include "Common/flags.h"

class QByteArray;
class QModelIndex;

namespace SpellCheck {
    class ISpellCheckable;

    class SpellSuggestionsItem: public QAbstractListModel
    {
        Q_OBJECT
    public:
        SpellSuggestionsItem(const QString &word,
                             const QString &origin);
        SpellSuggestionsItem(const QString &word);

    public:
        enum KeywordSpellSuggestions_Roles {
            SuggestionRole = Qt::UserRole + 1,
            // not same as inner m_IsSelected
            // used for selected replacement
            IsSelectedRole,
            EditReplacementIndexRole
        };

    public:
#if defined(CORE_TESTS) || defined(INTEGRATION_TESTS)
        virtual QString toDebugString() const { return QString("%1 -> (%2)").arg(m_Word).arg(m_Suggestions.join(", ")); }
#endif

    public:
        const QString &getWord() const { return m_Word; }
        const QString &getReplacement() const { return m_Suggestions.at(m_ReplacementIndex); }
        int getReplacementIndex() const { return m_ReplacementIndex; }
        const QString &getReplacementOrigin() const { return m_ReplacementOrigin; }
        bool getReplacementSucceeded() const { return m_ReplacementSucceeded; }
        bool anyReplacementSelected() const { return m_ReplacementIndex != -1; }

    public:
        bool setReplacementIndex(int value);
        void setSuggestions(const QStringList &suggestions);

    public:
        virtual void replaceToSuggested() = 0;

        // doesn't work like that because of f&cking c++ standard
        // about accessing base protected members in derived class
        // (you cannot access protected members of freestanding objects of base type)
        //protected:
        virtual void replaceToSuggested(const QString &word, const QString &replacement) = 0;

    signals:
        void replacementIndexChanged();

    public:
        virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
        virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
        virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
        virtual bool setData(const QModelIndex &index, const QVariant &value, int role) override;

    protected:
        virtual QHash<int, QByteArray> roleNames() const override;
        void setReplacementSucceeded(bool succeeded) { m_ReplacementSucceeded = succeeded; }
        const QStringList &getSuggestions() const { return m_Suggestions; }

    private:
        QStringList m_Suggestions;
        QString m_Word;
        QString m_ReplacementOrigin;
        int m_ReplacementIndex;
        bool m_ReplacementSucceeded;
    };

    class MetadataSpellSuggestionsItem: public SpellSuggestionsItem {
        Q_OBJECT
    public:
        MetadataSpellSuggestionsItem(const QString &word,
                                     const QString &origin,
                                     ISpellCheckable *spellCheckable);
        MetadataSpellSuggestionsItem(const QString &word,
                                     ISpellCheckable *spellCheckable);
    public:
        ISpellCheckable *getSpellCheckable() const { return m_SpellCheckable; }

    private:
        ISpellCheckable *m_SpellCheckable;
    };

    class KeywordSpellSuggestions: public MetadataSpellSuggestionsItem
    {
        Q_OBJECT
    public:
        KeywordSpellSuggestions(const QString &keyword,
                                size_t originalIndex,
                                const QString &origin,
                                ISpellCheckable *spellCheckable);
        KeywordSpellSuggestions(const QString &keyword,
                                size_t originalIndex,
                                ISpellCheckable *spellCheckable);

    public:
#if defined(CORE_TESTS) || defined(INTEGRATION_TESTS)
        virtual QString toDebugString() const override { return QString("KeywordReplace: %1 -> (%2)").arg(getWord()).arg(getSuggestions().join(", ")); }
#endif
        size_t getOriginalIndex() const { return m_OriginalIndex; }
        bool isPotentialDuplicate() const { return m_ReplaceResult == Common::KeywordReplaceResult::FailedDuplicate; }
        virtual void replaceToSuggested() override;

        // TODO: fix this back in future when c++ will be normal language (see comments in base class)
    //protected:
        virtual void replaceToSuggested(const QString &word, const QString &replacement) override;

    private:
        size_t m_OriginalIndex;
        Common::KeywordReplaceResult m_ReplaceResult;
    };

    class DescriptionSpellSuggestions: public MetadataSpellSuggestionsItem
    {
        Q_OBJECT
    public:
        DescriptionSpellSuggestions(const QString &word,
                                    ISpellCheckable *spellCheckable);

    public:
#if defined(CORE_TESTS) || defined(INTEGRATION_TESTS)
        virtual QString toDebugString() const override { return "DescReplace: " + SpellSuggestionsItem::toDebugString(); }
#endif
        virtual void replaceToSuggested() override;

    //protected:
        virtual void replaceToSuggested(const QString &word, const QString &replacement) override;
    };

    class TitleSpellSuggestions: public MetadataSpellSuggestionsItem
    {
        Q_OBJECT
    public:
        TitleSpellSuggestions(const QString &word,
                              ISpellCheckable *spellCheckable);

    public:
#if defined(CORE_TESTS) || defined(INTEGRATION_TESTS)
        virtual QString toDebugString() const override { return "TitleReplace: " + SpellSuggestionsItem::toDebugString(); }
#endif
        virtual void replaceToSuggested() override;

    //protected:
        virtual void replaceToSuggested(const QString &word, const QString &replacement) override;
    };

    class CombinedSpellSuggestions: public SpellSuggestionsItem {
        Q_OBJECT
    public:
        CombinedSpellSuggestions(const QString &word,
                                 std::vector<std::shared_ptr<SpellSuggestionsItem> > &suggestions);

    public:
#if defined(CORE_TESTS) || defined(INTEGRATION_TESTS)
        virtual QString toDebugString() const override { return "Multireplace: " + SpellSuggestionsItem::toDebugString(); }
#endif
        std::vector<std::shared_ptr<KeywordSpellSuggestions> > getKeywordsDuplicateSuggestions() const;
        virtual void replaceToSuggested() override;

    //protected:
        virtual void replaceToSuggested(const QString &word, const QString &replacement) override;

    private:
        std::vector<std::shared_ptr<SpellSuggestionsItem> > m_SpellSuggestions;
    };
}

Q_DECLARE_METATYPE(SpellCheck::KeywordSpellSuggestions*)

#endif // KEYWORDSPELLSUGGESTIONS_H

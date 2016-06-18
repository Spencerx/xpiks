/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2016 Taras Kushnir <kushnirTV@gmail.com>
 *
 * Xpiks is distributed under the GNU General Public License, version 3.0
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "spellsuggestionsitem.h"
#include "ispellcheckable.h"
#include "../Common/defines.h"

namespace SpellCheck {
    SpellSuggestionsItem::SpellSuggestionsItem(const QString &word, const QString &origin) :
        QAbstractListModel(),
        m_Word(word),
        m_ReplacementOrigin(origin),
        m_ReplacementIndex(-1),
        m_ReplacementSucceeded(false)
    {
    }

    SpellSuggestionsItem::SpellSuggestionsItem(const QString &word):
        QAbstractListModel(),
        m_Word(word),
        m_ReplacementOrigin(word),
        m_ReplacementIndex(-1),
        m_ReplacementSucceeded(false)
    {
    }

    bool SpellSuggestionsItem::setReplacementIndex(int value) {
        bool result = (value != m_ReplacementIndex) && (value != -1);

        if (result) {
            QVector<int> roles;
            roles << IsSelectedRole;
            int prevIndexToUpdate = m_ReplacementIndex;
            m_ReplacementIndex = value;

            if (m_ReplacementIndex != -1) {
                QModelIndex prev = this->index(prevIndexToUpdate);
                emit dataChanged(prev, prev, roles);
            }

            QModelIndex curr = this->index(value);
            emit dataChanged(curr, curr, roles);
        } else {
            if ((value == m_ReplacementIndex) ||
                    (value == -1 && m_ReplacementIndex != -1)) {
                int prevIndexToUpdate = m_ReplacementIndex;
                m_ReplacementIndex = -1;

                QModelIndex prev = this->index(prevIndexToUpdate);
                QVector<int> roles;
                roles << IsSelectedRole;
                emit dataChanged(prev, prev, roles);
                result = true;
            }
        }

        return result;
    }

    void SpellSuggestionsItem::setSuggestions(const QStringList &suggestions) {
        beginResetModel();
        m_Suggestions.clear();
        m_Suggestions.append(suggestions);
        endResetModel();
    }

    int SpellSuggestionsItem::rowCount(const QModelIndex &parent) const {
        Q_UNUSED(parent);
        return m_Suggestions.length();
    }

    QVariant SpellSuggestionsItem::data(const QModelIndex &index, int role) const {
        int row = index.row();
        if (row < 0 || row >= m_Suggestions.length()) { return QVariant(); }

        switch (role) {
        case SuggestionRole:
            return m_Suggestions.at(row);
        case IsSelectedRole:
            return (m_ReplacementIndex == row);
        default:
            return QVariant();
        }
    }

    Qt::ItemFlags SpellSuggestionsItem::flags(const QModelIndex &index) const {
        int row = index.row();
        if (row < 0 || row >= m_Suggestions.length()) {
            return Qt::ItemIsEnabled;
        }

        return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
    }

    bool SpellSuggestionsItem::setData(const QModelIndex &index, const QVariant &value, int role) {
        int row = index.row();
        if (row < 0 || row >= m_Suggestions.length()) return false;

        bool result = false;

        switch (role) {
        case EditReplacementIndexRole:
            result = setReplacementIndex(value.toInt());
            break;
        default:
            return false;
        }

        return result;
    }

    QHash<int, QByteArray> SpellSuggestionsItem::roleNames() const {
        QHash<int, QByteArray> roles;
        roles[SuggestionRole] = "suggestion";
        roles[IsSelectedRole] = "isselected";
        roles[EditReplacementIndexRole] = "editreplacementindex";
        return roles;
    }

    KeywordSpellSuggestions::KeywordSpellSuggestions(const QString &keyword, int originalIndex, const QString &origin) :
        SpellSuggestionsItem(keyword, origin),
        m_OriginalIndex(originalIndex),
        m_ReplaceResult(Common::KeywordReplaceUnknown)
    {
    }

    KeywordSpellSuggestions::KeywordSpellSuggestions(const QString &keyword, int originalIndex):
        SpellSuggestionsItem(keyword),
        m_OriginalIndex(originalIndex),
        m_ReplaceResult(Common::KeywordReplaceUnknown)
    {
    }

    void KeywordSpellSuggestions::replaceToSuggested(ISpellCheckable *item) {
        if (anyReplacementSelected()) {
            const QString &word = getWord();
            const QString &replacement = getReplacement();
            this->replaceToSuggested(item, word, replacement);
        }
    }

    void KeywordSpellSuggestions::replaceToSuggested(ISpellCheckable *item, const QString &word, const QString &replacement) {
        LOG_INFO << word << "-->" << replacement;
        Common::KeywordReplaceResult result = item->fixKeywordSpelling(m_OriginalIndex, word, replacement);
        setReplacementSucceeded(result == Common::KeywordReplaceSucceeded);
        m_ReplaceResult = result;
    }

    DescriptionSpellSuggestions::DescriptionSpellSuggestions(const QString &word):
        SpellSuggestionsItem(word)
    {
    }

    void DescriptionSpellSuggestions::replaceToSuggested(ISpellCheckable *item) {
        if (anyReplacementSelected()) {
            const QString &word = getWord();
            const QString &replacement = getReplacement();
            this->replaceToSuggested(item, word, replacement);
        }
    }

    void DescriptionSpellSuggestions::replaceToSuggested(ISpellCheckable *item, const QString &word, const QString &replacement) {
        LOG_INFO << word << "-->" << replacement;
        item->fixDescriptionSpelling(word, replacement);
        setReplacementSucceeded(true);
    }

    TitleSpellSuggestions::TitleSpellSuggestions(const QString &word):
        SpellSuggestionsItem(word)
    {
    }

    void TitleSpellSuggestions::replaceToSuggested(ISpellCheckable *item) {
        if (anyReplacementSelected()) {
            const QString &word = getWord();
            const QString &replacement = getReplacement();
            this->replaceToSuggested(item, word, replacement);
        }
    }

    void TitleSpellSuggestions::replaceToSuggested(ISpellCheckable *item, const QString &word, const QString &replacement) {
        LOG_INFO << word << "-->" << replacement;
        item->fixTitleSpelling(word, replacement);
        setReplacementSucceeded(true);
    }

    CombinedSpellSuggestions::CombinedSpellSuggestions(const QString &word, const QVector<SpellSuggestionsItem *> &suggestions):
        SpellSuggestionsItem(word, tr("multireplace")),
        m_SpellSuggestions(suggestions)
    {
        Q_ASSERT(!suggestions.isEmpty());
    }

    CombinedSpellSuggestions::~CombinedSpellSuggestions() {
        qDeleteAll(m_SpellSuggestions);
    }

    QVector<KeywordSpellSuggestions *> CombinedSpellSuggestions::getKeywordsDuplicateSuggestions() const {
        QVector<KeywordSpellSuggestions *> keywordsSuggestions;

        int size = m_SpellSuggestions.length();
        for (int i = 0; i < size; ++i) {
            SpellSuggestionsItem *suggestionItem = m_SpellSuggestions.at(i);
            KeywordSpellSuggestions *keywordsItem = dynamic_cast<KeywordSpellSuggestions *>(suggestionItem);
            if (keywordsItem != NULL && keywordsItem->isPotentialDuplicate()) {
                keywordsSuggestions.append(keywordsItem);
            }
        }

        return keywordsSuggestions;
    }

    void CombinedSpellSuggestions::replaceToSuggested(ISpellCheckable *item) {
        if (anyReplacementSelected()) {
            const QString &word = getWord();
            const QString &replacement = getReplacement();
            this->replaceToSuggested(item, word, replacement);
        }
    }

    void CombinedSpellSuggestions::replaceToSuggested(ISpellCheckable *item, const QString &word, const QString &replacement) {
        int size = m_SpellSuggestions.length();
        LOG_INFO << size << "item(s)";
        bool anyFault = false;

        for (int i = 0; i < size; ++i) {
            SpellSuggestionsItem *suggestionItem = m_SpellSuggestions.at(i);
            suggestionItem->replaceToSuggested(item, word, replacement);

#ifdef INTEGRATION_TESTS
            LOG_DEBUG << i << "item's result is:" << suggestionItem->getReplacementSucceeded();
#endif

            if (!suggestionItem->getReplacementSucceeded()) {
                anyFault = true;
            }
        }

        setReplacementSucceeded(!anyFault);
    }
}

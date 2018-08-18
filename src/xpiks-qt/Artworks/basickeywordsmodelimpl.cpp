/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "basickeywordsmodelimpl.h"
#include <QReadWriteLock>
#include <QReadLocker>
#include <QWriteLocker>
#include <algorithm>
#include <Services/SpellCheck/spellcheckitem.h>
#include <Services/SpellCheck/spellsuggestionsitem.h>
#include <Services/SpellCheck/spellcheckinfo.h>
#include <Helpers/keywordshelpers.h>
#include <Helpers/stringhelper.h>
#include <Helpers/indiceshelper.h>
#include <Common/logging.h>
#include <Common/flags.h>

namespace Artworks {
    BasicKeywordsModelImpl::BasicKeywordsModelImpl()
    {}

    QString BasicKeywordsModelImpl::getKeywordsString() {
        QStringList keywords = generateStringList();
        QString result = keywords.join(", ");
        return result;
    }

    bool BasicKeywordsModelImpl::appendKeyword(const QString &keyword, bool dryRun) {
        bool added = false;
        QString sanitizedKeyword = keyword.simplified();
        added = canBeAdded(sanitizedKeyword);

        if (added && !dryRun) {
            m_KeywordsSet.insert(sanitizedKeyword.toLower());
            m_KeywordsList.emplace_back(sanitizedKeyword);
            added = true;
        }

        return added;
    }

    void BasicKeywordsModelImpl::takeKeywordAt(size_t index, QString &removedKeyword, bool &wasCorrect) {
        Q_ASSERT(index < m_KeywordsList.size());
        const auto &keyword = m_KeywordsList.at(index);
        const QString invariant = keyword.m_Value.toLower();

        m_KeywordsSet.remove(invariant);

        wasCorrect = keyword.m_IsCorrect;
        removedKeyword = keyword.m_Value;

        m_KeywordsList.erase(m_KeywordsList.begin() + index);
    }

    bool BasicKeywordsModelImpl::prepareAppend(const QStringList &keywordsList, size_t &addedCount) {
        addedCount = appendKeywords(keywordsList, true);
        return addedCount > 0;
    }

    size_t BasicKeywordsModelImpl::appendKeywords(const QStringList &keywordsList, bool dryRun) {
        QStringList keywordsToAdd;
        int appendedCount = 0, size = keywordsList.length();

        keywordsToAdd.reserve(size);
        QSet<QString> accountedKeywords;

        for (int i = 0; i < size; ++i) {
            const QString &keyword = keywordsList.at(i);
            const QString &sanitizedKeyword = keyword.simplified();
            const QString &lowerCased = sanitizedKeyword.toLower();

            if (canBeAdded(sanitizedKeyword) && !accountedKeywords.contains(lowerCased)) {
                keywordsToAdd.append(sanitizedKeyword);
                accountedKeywords.insert(lowerCased);
                appendedCount++;
            }
        }

        size = keywordsToAdd.size();
        Q_ASSERT(size == appendedCount);

        if (!dryRun) {
            for (int i = 0; i < size; ++i) {
                const QString &keywordToAdd = keywordsToAdd.at(i);
                m_KeywordsSet.insert(keywordToAdd.toLower());
                m_KeywordsList.emplace_back(keywordToAdd);
            }
        }

        return appendedCount;
    }

    bool BasicKeywordsModelImpl::canEditKeyword(size_t index, const QString &replacement) const {
        Q_ASSERT(index < m_KeywordsList.size());
        bool result = false;
        LOG_INFO << "index:" << index << "replacement:" << replacement;

        QString sanitized = Helpers::doSanitizeKeyword(replacement);
        const QString existing = m_KeywordsList.at(index).m_Value;
        // IMPORTANT: keep track of copy-paste in editKeywordUnsafe()
        if (existing != sanitized && Helpers::isValidKeyword(sanitized)) {
            QString lowerCasedNew = sanitized.toLower();
            QString lowerCasedExisting = existing.toLower();

            if (!m_KeywordsSet.contains(lowerCasedNew)) {
                result = true;
            } else if (lowerCasedNew == lowerCasedExisting) {
                result = true;
            }
        }

        return result;
    }

    bool BasicKeywordsModelImpl::editKeyword(size_t index, const QString &replacement) {
        Q_ASSERT(index < m_KeywordsList.size());
        bool result = false;

        LOG_INFO << "index:" << index << "replacement:" << replacement;
        QString sanitized = Helpers::doSanitizeKeyword(replacement);

        auto &keyword = m_KeywordsList.at(index);
        QString existing = keyword.m_Value;
        // IMPORTANT: keep track of copy-paste in canEditKeywordUnsafe()
        if (existing != sanitized && Helpers::isValidKeyword(sanitized)) {
            QString lowerCasedNew = sanitized.toLower();
            QString lowerCasedExisting = existing.toLower();

            if (!m_KeywordsSet.contains(lowerCasedNew)) {
                m_KeywordsSet.insert(lowerCasedNew);
                m_KeywordsList[index].m_Value = sanitized;
                m_KeywordsSet.remove(lowerCasedExisting);
                LOG_INFO << "common case edit:" << existing << "->" << sanitized;

                result = true;
            } else if (lowerCasedNew == lowerCasedExisting) {
                LOG_INFO << "changing case in same keyword";
                m_KeywordsList[index].m_Value = sanitized;

                result = true;
            } else {
                LOG_WARNING << "Attempt to rename keyword to existing one. Use remove instead!";
            }
        }

        return result;
    }

    bool BasicKeywordsModelImpl::replaceKeyword(size_t index, const QString &existing, const QString &replacement) {
        Q_ASSERT(index < m_KeywordsList.size());
        bool result = false;

        auto &keyword = m_KeywordsList.at(index);
        const QString &internal = keyword.m_Value;

        if (internal == existing) {
            if (this->editKeyword(index, replacement)) {
                result = true;
            }
        } else if (internal.contains(existing) && internal.contains(QChar::Space)) {
            LOG_INFO << "Replacing composite keyword";
            QString existingFixed = internal;
            existingFixed.replace(existing, replacement);
            if (this->editKeyword(index, existingFixed)) {
                // TODO: reimplement this someday
                // no need to mark keyword as correct
                // if we replace only part of it
                result = true;
            }
        }

        return result;
    }

    bool BasicKeywordsModelImpl::moveKeyword(size_t from, size_t to) {
        const size_t size = m_KeywordsList.size();
        if (from >= size) { return false; }
        if (from == to) { return false; }
        if (to >= size) { to = size - 1; }

        if (from < to) {
            for (size_t i = from; i < to; i++) {
                m_KeywordsList[i].swap(m_KeywordsList[i + 1]);
            }
        } else /*if (from > to)*/ {
            for (size_t i = from; i > to; i--) {
                m_KeywordsList[i].swap(m_KeywordsList[i - 1]);
            }
        }

        return true;
    }

    bool BasicKeywordsModelImpl::clearKeywords() {
        const bool anyKeywords = !m_KeywordsList.empty();

        if (anyKeywords) {
            m_KeywordsList.clear();
            m_KeywordsSet.clear();
        } else {
            Q_ASSERT(m_KeywordsSet.empty());
        }

        return anyKeywords;
    }

    bool BasicKeywordsModelImpl::containsKeyword(const QString &searchTerm, Common::SearchFlags searchFlags) {
        bool hasMatch = false;
        const bool caseSensitive = Common::HasFlag(searchFlags, Common::SearchFlags::CaseSensitive);
        Qt::CaseSensitivity caseSensivity = caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive;
        const bool wholeWords = Common::HasFlag(searchFlags, Common::SearchFlags::WholeWords);

        if (wholeWords) {
            for (auto &keyword: m_KeywordsList) {
                if (QString::compare(keyword.m_Value, searchTerm, caseSensivity) == 0) {
                    hasMatch = true;
                    break;
                }
            }
        } else {
            for (auto &keyword: m_KeywordsList) {
                if (keyword.m_Value.contains(searchTerm, caseSensivity)) {
                    hasMatch = true;
                    break;
                }
            }
        }

        return hasMatch;
    }

    bool BasicKeywordsModelImpl::hasKeywordsSpellError() const {
        bool anyError = false;

        for (auto &keyword: m_KeywordsList) {
            if (!keyword.m_IsCorrect) {
                anyError = true;
                break;
            }
        }

        return anyError;
    }

    bool BasicKeywordsModelImpl::hasKeywordsDuplicates() const {
        bool hasDuplicate = false;

        for (auto &keyword: m_KeywordsList) {
            if (keyword.m_HasDuplicates) {
                hasDuplicate = true;
                break;
            }
        }

        return hasDuplicate;
    }

    bool BasicKeywordsModelImpl::findKeywordsIndices(const QSet<QString> &keywordsToFind, bool caseSensitive, std::vector<int> &foundIndices) {
        const size_t size = m_KeywordsList.size();

        foundIndices.reserve(size/2);

        for (size_t i = 0; i < size; ++i) {
            QString keyword = m_KeywordsList.at(i).m_Value;

            if (!caseSensitive) {
                keyword = keyword.toLower();
            }

            if (keywordsToFind.contains(keyword)) {
                foundIndices.push_back((int)i);
            }
        }

        bool anythingFound = !foundIndices.empty();
        return anythingFound;
    }

    bool BasicKeywordsModelImpl::hasKeywords(const QStringList &keywordsList) const {
        bool anyMissing = false;

        for (auto &item: keywordsList) {
            if (canBeAdded(item.simplified())) {
                anyMissing = true;
                break;
            }
        }

        return !anyMissing;
    }

    QStringList BasicKeywordsModelImpl::generateStringList() {
        QStringList result;
        result.reserve((int)m_KeywordsList.size());

        for (auto &keyword: m_KeywordsList) {
            result.append(keyword.m_Value);
        }

        return result;
    }

    std::vector<KeywordItem> BasicKeywordsModelImpl::retrieveMisspelledKeywords() {
        std::vector<KeywordItem> misspelledKeywords;
        size_t size = m_KeywordsList.size();
        misspelledKeywords.reserve(size/3);

        for (size_t i = 0; i < size; ++i) {
            auto &item = m_KeywordsList.at(i);
            if (!item.m_IsCorrect) {
                const QString &keyword = item.m_Value;
                LOG_INTEGR_TESTS_OR_DEBUG << keyword << "has wrong spelling";

                if (!keyword.contains(QChar::Space)) {
                    misspelledKeywords.emplace_back(keyword, i);
                } else {
                    QStringList items = keyword.split(QChar::Space, QString::SkipEmptyParts);
                    foreach(const QString &item, items) {
                        misspelledKeywords.emplace_back(item, i, keyword);
                    }
                }
            }
        }

        return misspelledKeywords;
    }

    std::vector<KeywordItem> BasicKeywordsModelImpl::retrieveDuplicatedKeywords() {
        std::vector<KeywordItem> duplicatedKeywords;
        size_t size = m_KeywordsList.size();
        duplicatedKeywords.reserve(size/3);

        for (size_t i = 0; i < size; ++i) {
            auto &item = m_KeywordsList.at(i);
            if (item.m_HasDuplicates) {
                duplicatedKeywords.emplace_back(item.m_Value, i);
            }
        }

        return duplicatedKeywords;
    }

    Common::KeywordReplaceResult BasicKeywordsModelImpl::fixKeywordSpelling(size_t index, const QString &existing, const QString &replacement) {
        Common::KeywordReplaceResult result;

        LOG_INFO << "Replacing" << existing << "to" << replacement << "with index" << index;

        const size_t size = m_KeywordsList.size();
        if (index < size) {
            if (replaceKeyword(index, existing, replacement)) {
                m_KeywordsList[index].m_IsCorrect = true;
                result = Common::KeywordReplaceResult::Succeeded;
            } else {
                result = Common::KeywordReplaceResult::FailedDuplicate;
            }
        } else {
            LOG_INFO << "Failure. Index is negative or exceeds count" << size;
            result = Common::KeywordReplaceResult::FailedIndex;
        }

        return result;
    }

    bool BasicKeywordsModelImpl::processFailedKeywordReplacements(const std::vector<std::shared_ptr<SpellCheck::KeywordSpellSuggestions> > &candidatesForRemoval,
                                                                  std::vector<int> &indicesToRemove) {
        LOG_INFO << candidatesForRemoval.size() << "candidates to remove";
        bool anyReplaced = false;

        if (candidatesForRemoval.empty()) {
            return anyReplaced;
        }

        const size_t size = candidatesForRemoval.size();
        indicesToRemove.reserve(size);

        for (size_t i = 0; i < size; ++i) {
            auto &item = candidatesForRemoval.at(i);

            size_t index = item->getOriginalIndex();
            if (index >= m_KeywordsList.size()) {
                LOG_DEBUG << "index is out of range";
                continue;
            }

            const QString &existingPrev = item->getWord();
            QString sanitized = Helpers::doSanitizeKeyword(item->getReplacement());

            if (isReplacedADuplicate(index, existingPrev, sanitized)) {
                indicesToRemove.push_back((int)index);
            }
        }

        LOG_INFO << "confirmed" << indicesToRemove.size() << "duplicates to remove";

        if (!indicesToRemove.empty()) {
            anyReplaced = true;
        }

        return anyReplaced;
    }

    bool BasicKeywordsModelImpl::replaceInKeywords(const QString &replaceWhat, const QString &replaceTo,
                                                         Common::SearchFlags flags, QVector<int> &indicesToRemove,
                                                         QVector<int> &indicesToUpdate) {
        bool anyChanged = false;

        const bool caseSensitive = Common::HasFlag(flags, Common::SearchFlags::CaseSensitive);
        const bool wholeWords = Common::HasFlag(flags, Common::SearchFlags::WholeWords);
        const Qt::CaseSensitivity caseSensivity = caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive;

        const size_t size = m_KeywordsList.size();
        for (size_t i = 0; i < size; ++i) {
            auto &keyword = m_KeywordsList.at(i);
            QString internal = keyword.m_Value;
            const bool hasMatch = wholeWords ?
                                  Helpers::containsWholeWords(internal, replaceWhat, caseSensivity) :
                                  internal.contains(replaceWhat, caseSensivity);
            LOG_FOR_TESTS << "[" << internal << "] has match [" << replaceWhat << "] =" << hasMatch;

            if (hasMatch) {
                QString replaced = wholeWords ?
                                   Helpers::replaceWholeWords(internal, replaceWhat, replaceTo, caseSensivity) :
                                   internal.replace(replaceWhat, replaceTo, caseSensivity);

                QString replacement = Helpers::doSanitizeKeyword(replaced);

                if (!this->editKeyword(i, replacement)) {
                    if (replacement.isEmpty()) {
                        LOG_INFO << "Replaced" << internal << "to empty";
                        indicesToRemove.append((int)i);
                    } else if (m_KeywordsSet.contains(replacement.toLower())) {
                        LOG_INFO << "Replacing" << internal << "to" << replacement << "creates a duplicate";
                        indicesToRemove.append((int)i);
                    }
                } else {
                    indicesToUpdate.append((int)i);
                    anyChanged = true;
                }
            }
        }

        if (!indicesToRemove.isEmpty()) {
            anyChanged = true;
        }

        return anyChanged;
    }

    bool BasicKeywordsModelImpl::containsKeywords(const QStringList &keywordsList) {
        bool anyError = false;

        Common::SearchFlags searchFlags = Common::SearchFlags::ExactKeywords;
        for (auto &keyword: keywordsList) {
            if (!containsKeyword(keyword, searchFlags)) {
                anyError = true;
                break;
            }
        }

        return !anyError;
    }

    void BasicKeywordsModelImpl::resetSpellCheckResults() {
        for (auto &keyword: m_KeywordsList) {
            keyword.m_IsCorrect = true;
        }
    }

    void BasicKeywordsModelImpl::resetDuplicatesInfo() {
        for (auto &keyword: m_KeywordsList) {
            keyword.m_HasDuplicates = false;
        }
    }

    bool BasicKeywordsModelImpl::canBeAdded(const QString &keyword) const {
        bool isValid = Helpers::isValidKeyword(keyword);
        bool result = isValid && !m_KeywordsSet.contains(keyword.toLower());

        return result;
    }

    bool BasicKeywordsModelImpl::hasKeyword(const QString &keyword) {
        return !canBeAdded(keyword.simplified());
    }

    void BasicKeywordsModelImpl::setSpellCheckResults(const std::vector<std::shared_ptr<SpellCheck::SpellCheckQueryItem> > &items) {
        const size_t itemsSize = items.size();
        const size_t keywordsSize = m_KeywordsList.size();

        // reset relative items
        for (size_t i = 0; i < itemsSize; ++i) {
            auto &item = items.at(i);
            size_t index = item->m_Index;
            if (index < keywordsSize) {
                m_KeywordsList[index].resetSpelling();
            }

            if (index >= keywordsSize) {
                LOG_DEBUG << "Skipping the rest of overflowing results";
#ifdef QT_DEBUG
                // if any of these has overflowing index, then all of them should
                for (size_t j = i; j < itemsSize; ++j) {
                    Q_ASSERT(items.at(j)->m_Index >= keywordsSize);
                }
#endif
                break;
            }
        }

        for (size_t i = 0; i < itemsSize; ++i) {
            auto &item = items.at(i);
            size_t index = item->m_Index;
            // looks like this is a stupid assert to trace impossible race conditions
            Q_ASSERT(keywordsSize == m_KeywordsList.size());

            if (index < keywordsSize) {
                auto &keyword = m_KeywordsList[index];
                if (keyword.m_Value.contains(item->m_Word)) {
                    // if keyword contains several words, there would be
                    // several queryiIems and there's error if any has error
                    keyword.m_IsCorrect = keyword.m_IsCorrect && item->m_IsCorrect;
                    keyword.m_HasDuplicates = keyword.m_HasDuplicates || item->m_IsDuplicate;
                }
            }

            if (index >= keywordsSize) {
                LOG_DEBUG << "Skipping the rest of overflowing results";
                break;
            }
        }
    }

    bool BasicKeywordsModelImpl::isReplacedADuplicate(size_t index, const QString &existingPrev,
                                                      const QString &replacement) const {
        bool isDuplicate = false;
        const QString &existingCurrent = m_KeywordsList.at(index).m_Value;

        if (existingCurrent == existingPrev) {
            if (m_KeywordsSet.contains(replacement.toLower())) {
                isDuplicate = true;
                LOG_INFO << "safe to remove duplicate [" << existingCurrent << "] at index" << index;
            } else {
                LOG_INFO << replacement << "was not found";
            }
        } else if (existingCurrent.contains(existingPrev) && existingCurrent.contains(QChar::Space)) {
            QString existingFixed = existingCurrent;
            existingFixed.replace(existingPrev, replacement);

            if (m_KeywordsSet.contains(existingFixed.toLower())) {
                isDuplicate = true;
                LOG_INFO << "safe to remove composite duplicate [" << existingCurrent << "] at index" << index;
            } else {
                LOG_INFO << existingFixed << "was not found";
            }
        } else {
            LOG_INFO << existingCurrent << "is now instead of" << existingPrev << "at index" << index;
        }

        return isDuplicate;
    }
}

/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "basickeywordsmodel.h"
#include <QReadWriteLock>
#include <QReadLocker>
#include <QWriteLocker>
#include "../SpellCheck/spellcheckitem.h"
#include "../SpellCheck/spellsuggestionsitem.h"
#include "../SpellCheck/spellcheckiteminfo.h"
#include "../Helpers/keywordshelpers.h"
#include "../Helpers/stringhelper.h"
#include "flags.h"
#include "defines.h"
#include "../Helpers/indiceshelper.h"
#include "basickeywordsmodelimpl.h"

namespace Common {
    BasicKeywordsModel::BasicKeywordsModel(Hold &hold, QObject *parent):
        AbstractListModel(parent),
        m_Impl(new BasicKeywordsModelImpl(hold))
    {}

#ifdef CORE_TESTS
    const QString &BasicKeywordsModel::getKeywordAt(int index) const { return m_Impl->accessKeyword(index).m_Value; }
    std::vector<Keyword> &BasicKeywordsModel::getRawKeywords() { return m_Impl->m_KeywordsList; }
#endif

#ifdef INTEGRATION_TESTS
        bool BasicKeywordsModel::hasDuplicateAt(size_t i) const { return m_Impl->accessKeyword(i).m_HasDuplicates; }
#endif

    void BasicKeywordsModel::removeItemsFromRanges(const QVector<QPair<int, int> > &ranges) {
        LOG_INFO << "#";

        QWriteLocker writeLocker(&m_KeywordsLock);
        Q_UNUSED(writeLocker);

        AbstractListModel::removeItemsFromRanges(ranges);
    }

    void BasicKeywordsModel::removeInnerItem(int row) {
        QString removedKeyword;
        bool wasCorrect = false;

        m_Impl->takeKeywordAt(row, removedKeyword, wasCorrect);
        LOG_INTEGRATION_TESTS << "keyword:" << removedKeyword << "was correct:" << wasCorrect;
        Q_UNUSED(removedKeyword);
        Q_UNUSED(wasCorrect);
    }

    int BasicKeywordsModel::rowCount(const QModelIndex &parent) const {
        Q_UNUSED(parent);
        return (int)m_Impl->getKeywordsSize();
    }

    QVariant BasicKeywordsModel::data(const QModelIndex &index, int role) const {
        //QReadLocker readLocker(&m_KeywordsLock);
        //Q_UNUSED(readLocker);

        const int row = index.row();

        if (row < 0 || row >= (int)m_Impl->getKeywordsSize()) {
            return QVariant();
        }

        auto &keyword = m_Impl->accessKeyword(row);

        switch (role) {
            case KeywordRole:
                return keyword.m_Value;
            case IsCorrectRole:
                return keyword.m_IsCorrect;
            case HasDuplicateRole:
                return keyword.m_HasDuplicates;
            default:
                return QVariant();
        }
    }

    QHash<int, QByteArray> BasicKeywordsModel::roleNames() const {
        QHash<int, QByteArray> roles;
        roles[KeywordRole] = "keyword";
        roles[IsCorrectRole] = "iscorrect";
        roles[HasDuplicateRole] = "hasduplicate";
        return roles;
    }

    int BasicKeywordsModel::getKeywordsCount() {
        QReadLocker readLocker(&m_KeywordsLock);
        Q_UNUSED(readLocker);
        return (int)m_Impl->getKeywordsSize();
    }

    QSet<QString> BasicKeywordsModel::getKeywordsSet() {
        QReadLocker readLocker(&m_KeywordsLock);
        Q_UNUSED(readLocker);
        return m_Impl->getKeywordsSet();
    }

    QString BasicKeywordsModel::getKeywordsString() {
        QReadLocker readLocker(&m_KeywordsLock);
        Q_UNUSED(readLocker);
        return m_Impl->getKeywordsString();
    }

    bool BasicKeywordsModel::appendKeyword(const QString &keyword) {
        bool added = false;

        QWriteLocker locker(&m_KeywordsLock);
        Q_UNUSED(locker);

        bool dryRun = true;
        if (m_Impl->appendKeyword(keyword, dryRun)) {
            size_t index = m_Impl->getKeywordsSize();

            beginInsertRows(QModelIndex(), (int)index, (int)index);
            m_Impl->appendKeyword(keyword);
            endInsertRows();

            added = true;
        }

        return added;
    }

    bool BasicKeywordsModel::removeKeywordAt(size_t index, QString &removedKeyword) {
        bool wasCorrect = false, removed = false;

        m_KeywordsLock.lockForWrite();
        {
            if (index < m_Impl->getKeywordsSize()) {
                beginRemoveRows(QModelIndex(), (int)index, (int)index);
                m_Impl->takeKeywordAt(index, removedKeyword, wasCorrect);
                endRemoveRows();
                removed = true;
            }
        }
        m_KeywordsLock.unlock();

        if (removed) {
            if (!wasCorrect) {
                notifyKeywordsSpellingChanged();
            }

            emit hasDuplicatesChanged();
        }

        return removed;
    }

    bool BasicKeywordsModel::removeLastKeyword(QString &removedKeyword) {
        bool wasCorrect = false, removed = false;
        size_t indexLast = 0;

        m_KeywordsLock.lockForWrite();
        {
            const size_t size = m_Impl->getKeywordsSize();
            if (size > 0) {
                indexLast = size - 1;
                beginRemoveRows(QModelIndex(), (int)indexLast, (int)indexLast);
                m_Impl->takeKeywordAt(indexLast, removedKeyword, wasCorrect);
                endRemoveRows();
                removed = true;
            }
        }
        m_KeywordsLock.unlock();

        if (removed) {
            if (!wasCorrect) {
                notifyKeywordsSpellingChanged();
            }

            emit hasDuplicatesChanged();
        }

        return removed;
    }

    void BasicKeywordsModel::setKeywords(const QStringList &keywordsList) {
        beginResetModel();
        {
            QWriteLocker writeLocker(&m_KeywordsLock);
            Q_UNUSED(writeLocker);

            m_Impl->clearKeywords();
            m_Impl->appendKeywords(keywordsList);
        }
        endResetModel();
    }

    size_t BasicKeywordsModel::appendKeywords(const QStringList &keywordsList) {
        QWriteLocker writeLocker(&m_KeywordsLock);
        Q_UNUSED(writeLocker);

        return appendKeywordsUnsafe(keywordsList);
    }

    bool BasicKeywordsModel::editKeyword(size_t index, const QString &replacement) {
        bool result = false;

        m_KeywordsLock.lockForWrite();
        {
            if (index < m_Impl->getKeywordsSize()) {
                result = m_Impl->editKeyword(index, replacement);
            } else {
                LOG_WARNING << "Failed to edit keyword with index" << index;
            }
        }
        m_KeywordsLock.unlock();

        if (result) {
            QModelIndex i = this->index((int)index);
            emit dataChanged(i, i, QVector<int>() << KeywordRole);
        }

        return result;
    }

    bool BasicKeywordsModel::clearKeywords() {
        bool result = false;

        beginResetModel();
        {
            QWriteLocker locker(&m_KeywordsLock);
            Q_UNUSED(locker);

            result = m_Impl->clearKeywords();
        }
        endResetModel();

        if (result) {
            notifyKeywordsSpellingChanged();
            emit hasDuplicatesChanged();
        }

        return result;
    }

    bool BasicKeywordsModel::expandPreset(size_t keywordIndex, const QStringList &presetList) {
        Q_ASSERT(!presetList.isEmpty());
        LOG_INFO << keywordIndex;
        bool expanded = false;
        size_t addedCount = 0;

        {
            QWriteLocker writeLocker(&m_KeywordsLock);
            Q_UNUSED(writeLocker);

            if (keywordIndex < m_Impl->getKeywordsSize()) {
                LOG_INFO << "index" << keywordIndex << "list:" << presetList;
                QString removedKeyword;
                bool wasCorrect = false;
                beginRemoveRows(QModelIndex(), (int)keywordIndex, (int)keywordIndex);
                m_Impl->takeKeywordAt(keywordIndex, removedKeyword, wasCorrect);
                endRemoveRows();

                LOG_INFO << "replaced keyword" << removedKeyword;
                Q_UNUSED(wasCorrect);

                addedCount = appendKeywordsUnsafe(presetList);
                expanded = addedCount > 0;
            }
        }

        return expanded;
    }

    bool BasicKeywordsModel::appendPreset(const QStringList &presetList) {
        LOG_DEBUG << "#";
        bool result = appendKeywords(presetList) > 0;
        return result;
    }

    bool BasicKeywordsModel::hasKeywords(const QStringList &keywordsList) {
        QReadLocker locker(&m_KeywordsLock);
        Q_UNUSED(locker);
        return m_Impl->hasKeywords(keywordsList);
    }

    bool BasicKeywordsModel::areKeywordsEmpty() {
        QReadLocker locker(&m_KeywordsLock);
        Q_UNUSED(locker);
        return m_Impl->areKeywordsEmpty();
    }

    bool BasicKeywordsModel::replace(const QString &replaceWhat, const QString &replaceTo, Common::SearchFlags flags) {
        LOG_INTEGR_TESTS_OR_DEBUG << replaceWhat << "->" << replaceTo << "with flags:" << (int)flags;
        Q_ASSERT(!replaceWhat.isEmpty());
        Q_ASSERT(((int)flags & (int)Common::SearchFlags::Metadata) != 0);

        bool anyChanged = false;
        QVector<int> indicesToUpdate, indicesToRemove;
        QVector<QPair<int, int> > rangesToRemove;

        const bool needToCheckKeywords = Common::HasFlag(flags, Common::SearchFlags::Keywords);
        if (needToCheckKeywords) {
            QWriteLocker locker(&m_KeywordsLock);
            Q_UNUSED(locker);

            if (m_Impl->replaceInKeywords(replaceWhat, replaceTo, flags, indicesToRemove, indicesToUpdate)) {
                Helpers::indicesToRanges(indicesToRemove, rangesToRemove);
                AbstractListModel::removeItemsFromRanges(rangesToRemove);
                anyChanged = true;
            }
        }

        if (anyChanged) {
            const int size = (int)m_Impl->getKeywordsSize();
            // update all because of easiness instead of calculating correct shifts after removal
            this->updateItemsInRanges({{0, size - 1}}, QVector<int>() << KeywordRole);
        }

        return anyChanged;
    }

    bool BasicKeywordsModel::removeKeywords(const QSet<QString> &keywords, bool caseSensitive) {
        bool anyRemoved = false;

        QVector<QPair<int, int> > rangesToRemove;

        {
            QWriteLocker locker(&m_KeywordsLock);
            Q_UNUSED(locker);

            QVector<int> indicesToRemove;
            if (m_Impl->findKeywordsIndices(keywords, caseSensitive, indicesToRemove)) {
                Helpers::indicesToRanges(indicesToRemove, rangesToRemove);
                AbstractListModel::removeItemsFromRanges(rangesToRemove);
                anyRemoved = true;
            }
        }

        return anyRemoved;
    }

    size_t BasicKeywordsModel::appendKeywordsUnsafe(const QStringList &keywordsList) {
        size_t appendedCount = 0;
        if (m_Impl->prepareAppend(keywordsList, appendedCount)) {
            const size_t size = m_Impl->getKeywordsSize();
            beginInsertRows(QModelIndex(), (int)size, (int)(size + appendedCount - 1));
            m_Impl->appendKeywords(keywordsList);
            endInsertRows();
        }

        return appendedCount;
    }

    QString BasicKeywordsModel::retrieveKeyword(size_t wordIndex) {
        QReadLocker locker(&m_KeywordsLock);
        Q_UNUSED(locker);
        auto &keyword = m_Impl->accessKeyword(wordIndex);
        return keyword.m_Value;
    }

    QStringList BasicKeywordsModel::getKeywords() {
        QReadLocker locker(&m_KeywordsLock);
        Q_UNUSED(locker);
        return m_Impl->generateStringList();
    }

    void BasicKeywordsModel::setKeywordsSpellCheckResults(const std::vector<std::shared_ptr<SpellCheck::SpellCheckQueryItem> > &items) {
        QWriteLocker locker(&m_KeywordsLock);
        Q_UNUSED(locker);
        m_Impl->setSpellCheckResults(items);
    }

    std::vector<KeywordItem> BasicKeywordsModel::retrieveMisspelledKeywords() {
        QReadLocker locker(&m_KeywordsLock);
        Q_UNUSED(locker);
        return m_Impl->retrieveMisspelledKeywords();
    }

    std::vector<KeywordItem> BasicKeywordsModel::retrieveDuplicatedKeywords() {
        QReadLocker locker(&m_KeywordsLock);
        Q_UNUSED(locker);
        return m_Impl->retrieveDuplicatedKeywords();
    }

    Common::KeywordReplaceResult BasicKeywordsModel::fixKeywordSpelling(size_t index, const QString &existing, const QString &replacement) {
        Common::KeywordReplaceResult result;

        {
            QWriteLocker locker(&m_KeywordsLock);
            Q_UNUSED(locker);
            result = m_Impl->fixKeywordSpelling(index, existing, replacement);
        }

        if (result == Common::KeywordReplaceResult::Succeeded) {
            QModelIndex i = this->index((int)index);
            // combined roles from legacy editKeyword() and replace()
            emit dataChanged(i, i, QVector<int>() << KeywordRole << IsCorrectRole << HasDuplicateRole);
        }

        return result;
    }

    bool BasicKeywordsModel::processFailedKeywordReplacements(const
                                                              std::vector<std::shared_ptr<SpellCheck::KeywordSpellSuggestions> > &candidatesForRemoval)
    {
        bool anyReplaced = false;
        QVector<QPair<int, int> > rangesToRemove;

        {
            QWriteLocker locker(&m_KeywordsLock);
            Q_UNUSED(locker);

            QVector<int> indicesToRemove;
            if (m_Impl->processFailedKeywordReplacements(candidatesForRemoval, indicesToRemove)) {
                Q_ASSERT(!indicesToRemove.isEmpty());
                Helpers::indicesToRanges(indicesToRemove, rangesToRemove);
                AbstractListModel::removeItemsFromRanges(rangesToRemove);
                anyReplaced = true;
            }
        }

        return anyReplaced;
    }

    void BasicKeywordsModel::connectSignals(SpellCheck::SpellCheckItem *item) {
        QObject::connect(item, &SpellCheck::SpellCheckItem::resultsReady,
                         this, &BasicKeywordsModel::onSpellCheckRequestReady);
    }

    void BasicKeywordsModel::afterReplaceCallback() {
        LOG_DEBUG << "#";
        emit notifyKeywordsSpellingChanged();
        emit afterSpellingErrorsFixed();
        emit hasDuplicatesChanged();
        emit spellingInfoUpdated();
    }

    void BasicKeywordsModel::resetSpellCheckResults() {
        {
            QWriteLocker locker(&m_KeywordsLock);
            Q_UNUSED(locker);
            m_Impl->resetSpellCheckResults();
        }
        emit spellingInfoUpdated();
    }

    void BasicKeywordsModel::resetDuplicatesInfo() {
        {
            QWriteLocker locker(&m_KeywordsLock);
            Q_UNUSED(locker);
            m_Impl->resetDuplicatesInfo();
        }
        emit spellingInfoUpdated();
    }

    bool BasicKeywordsModel::containsKeyword(const QString &searchTerm, Common::SearchFlags searchFlags) {
        QReadLocker locker(&m_KeywordsLock);
        Q_UNUSED(locker);
        return m_Impl->containsKeyword(searchTerm, searchFlags);
    }

    bool BasicKeywordsModel::containsKeywords(const QStringList &keywordsList) {
        QReadLocker locker(&m_KeywordsLock);
        Q_UNUSED(locker);
        return m_Impl->containsKeywords(keywordsList);
    }

    bool BasicKeywordsModel::isEmpty() {
        QReadLocker locker(&m_KeywordsLock);
        Q_UNUSED(locker);
        return m_Impl->areKeywordsEmpty();
    }

    bool BasicKeywordsModel::hasKeywordsSpellError() {
        QReadLocker locker(&m_KeywordsLock);
        Q_UNUSED(locker);
        return m_Impl->hasKeywordsSpellError();
    }

    bool BasicKeywordsModel::hasKeywordsDuplicates() {
        QReadLocker locker(&m_KeywordsLock);
        Q_UNUSED(locker);
        return m_Impl->hasKeywordsDuplicates();
    }

    bool BasicKeywordsModel::hasSpellErrors() {
        QReadLocker locker(&m_KeywordsLock);
        Q_UNUSED(locker);
        return m_Impl->hasKeywordsSpellError();
    }

    bool BasicKeywordsModel::hasDuplicates() {
        QReadLocker locker(&m_KeywordsLock);
        Q_UNUSED(locker);
        return m_Impl->hasKeywordsDuplicates();
    }

    void BasicKeywordsModel::notifySpellCheckResults(Common::SpellCheckFlags flags) {
        if (Common::HasFlag(flags, Common::SpellCheckFlags::Keywords)) {
            updateKeywordsHighlighting();
            notifyKeywordsSpellingChanged();
        }

        emit hasDuplicatesChanged();
    }

    void BasicKeywordsModel::notifyKeywordsSpellingChanged() {
        emit keywordsSpellingChanged();
    }

    void BasicKeywordsModel::acquire() {
        m_Impl->acquire();
    }

    bool BasicKeywordsModel::release() {
        return m_Impl->release();
    }

    bool BasicKeywordsModel::hasKeyword(const QString &keyword) {
        QReadLocker locker(&m_KeywordsLock);
        Q_UNUSED(locker);
        return m_Impl->hasKeyword(keyword);
    }

    bool BasicKeywordsModel::canEditKeyword(int index, const QString &replacement) {
        QReadLocker locker(&m_KeywordsLock);
        Q_UNUSED(locker);
        return m_Impl->canEditKeyword(index, replacement);
    }

    void BasicKeywordsModel::onSpellCheckRequestReady(Common::SpellCheckFlags flags, int index) {
        notifySpellCheckResults(flags);
        Q_UNUSED(index);
        emit spellingInfoUpdated();
    }

    void BasicKeywordsModel::updateKeywordsHighlighting() {
        const int count = getKeywordsCount();

        if (count > 0) {
            QModelIndex start = this->index(0);
            QModelIndex end = this->index(count - 1);
            emit dataChanged(start, end, QVector<int>() << IsCorrectRole << HasDuplicateRole);
        }
    }
}

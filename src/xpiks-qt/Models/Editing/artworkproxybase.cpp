/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "artworkproxybase.h"
#include <Artworks/imetadataoperator.h>
#include <Suggestion/keywordssuggestor.h>
#include <QuickBuffer/quickbuffer.h>
#include <Helpers/stringhelper.h>
#include <AutoComplete/completionitem.h>
#include <AutoComplete/keywordsautocompletemodel.h>

namespace Models {
    ArtworkProxyBase::ArtworkProxyBase():
        m_CompletionSource(completionSource),
        m_PresetsManager(presetsManager)
    {
    }

    QString ArtworkProxyBase::getDescription() {
        auto *basicModel = getBasicMetadataModel();
        return basicModel->getDescription();
    }

    QString ArtworkProxyBase::getTitle() {
        auto *basicModel = getBasicMetadataModel();
        return basicModel->getTitle();
    }

    QStringList ArtworkProxyBase::getKeywords() {
        auto *basicModel = getBasicMetadataModel();
        return basicModel->getKeywords();
    }

    int ArtworkProxyBase::getKeywordsCount() {
        auto *basicModel = getBasicMetadataModel();
        return basicModel->getKeywordsCount();
    }

    void ArtworkProxyBase::setDescription(const QString &description) {
        if (doSetDescription(description)) {
            signalDescriptionChanged();
            doJustEdited();
        }
    }

    void ArtworkProxyBase::setTitle(const QString &title) {
        if (doSetTitle(title)) {
            signalTitleChanged();
            doJustEdited();
        }
    }

    void ArtworkProxyBase::setKeywords(const QStringList &keywords) {
        doSetKeywords(keywords);
        signalKeywordsCountChanged();
        doJustEdited();
        submitForInspection();
    }

    bool ArtworkProxyBase::doSetDescription(const QString &description) {
        auto *metadataOperator = getMetadataOperator();
        bool result = metadataOperator->setDescription(description);
        if (result) {
            doJustEdited();
            submitForInspection();
        }

        return result;
    }

    bool ArtworkProxyBase::doSetTitle(const QString &title) {
        auto *metadataOperator = getMetadataOperator();
        bool result = metadataOperator->setTitle(title);
        if (result) {
            doJustEdited();
            submitForInspection();
        }

        return result;
    }

    void ArtworkProxyBase::doSetKeywords(const QStringList &keywords) {
        auto *metadataOperator = getMetadataOperator();
        metadataOperator->setKeywords(keywords);
        doJustEdited();
        submitForInspection();
    }

    Common::ID_t ArtworkProxyBase::getSpecialItemID() {
        return SPECIAL_ID_INVALID;
    }

    bool ArtworkProxyBase::doEditKeyword(int index, const QString &replacement) {
        LOG_INFO << "index:" << index << "replacement:" << replacement;
        auto *metadataOperator = getMetadataOperator();
        bool result = metadataOperator->editKeyword(index, replacement);
        if (result) {
            submitForInspection();
            doJustEdited();
        } else {
            LOG_INFO << "Failed to edit to" << replacement;
        }

        return result;
    }

    bool ArtworkProxyBase::doRemoveKeywordAt(int index, QString &keyword) {
        LOG_INFO << "keyword index:" << index;
        auto *metadataOperator = getMetadataOperator();
        bool result = metadataOperator->removeKeywordAt(index, keyword);
        if (result) {
            LOG_INFO << "Removed keyword:" << keyword << "keywords count:" << getKeywordsCount();
            signalKeywordsCountChanged();
            submitForInspection();
            doJustEdited();
        }

        return result;
    }

    bool ArtworkProxyBase::doRemoveLastKeyword(QString &keyword) {
        LOG_DEBUG << "#";
        auto *metadataOperator = getMetadataOperator();
        bool result = metadataOperator->removeLastKeyword(keyword);
        if (result) {
            LOG_INFO << "Removed keyword:" << keyword << "keywords count:" << getKeywordsCount();
            signalKeywordsCountChanged();
            submitForInspection();
            doJustEdited();
        }

        return result;
    }

    bool ArtworkProxyBase::doAppendKeyword(const QString &keyword) {
        LOG_INFO << keyword;
        auto *metadataOperator = getMetadataOperator();
        bool result = metadataOperator->appendKeyword(keyword);
        if (result) {
            signalKeywordsCountChanged();
            submitForInspection();
            doJustEdited();
        } else {
            LOG_INFO << "Failed to append:" << keyword;
        }

        return result;
    }

    size_t ArtworkProxyBase::doAppendKeywords(const QStringList &keywords) {
        LOG_INFO << keywords.length() << "keyword(s)" << "|" << keywords;
        auto *metadataOperator = getMetadataOperator();
        const size_t appendedCount = metadataOperator->appendKeywords(keywords);
        LOG_INFO << "Appended" << appendedCount << "keywords";

        if (appendedCount > 0) {
            signalKeywordsCountChanged();
            submitForInspection();
            doJustEdited();
        }

        return appendedCount;
    }

    bool ArtworkProxyBase::doRemoveKeywords(const QSet<QString> &keywords, bool caseSensitive) {
        LOG_INFO << "case sensitive:" << caseSensitive;
        auto *metadataOperator = getMetadataOperator();
        const bool result = metadataOperator->removeKeywords(keywords, caseSensitive);
        if (result) {
            signalKeywordsCountChanged();
            submitForInspection();
            doJustEdited();
        }

        return result;
    }

    bool ArtworkProxyBase::doMoveKeyword(int from, int to) {
        LOG_INFO << from << "-->" << to;
        auto *metadataOperator = getMetadataOperator();
        const bool result = metadataOperator->moveKeyword(from, to);
        if (result) {
            doJustEdited();
        }

        return result;
    }

    bool ArtworkProxyBase::doClearKeywords() {
        LOG_DEBUG << "#";
        auto *metadataOperator = getMetadataOperator();
        const bool result = metadataOperator->clearKeywords();
        if (result) {
            signalKeywordsCountChanged();
            submitForInspection();
            doJustEdited();
        }

        // do not request backup

        return result;
    }

    QString ArtworkProxyBase::doGetKeywordsString() {
        auto *metadataOperator = getMetadataOperator();
        return metadataOperator->getKeywordsString();
    }

    void ArtworkProxyBase::doPlainTextEdit(const QString &rawKeywords, bool spaceIsSeparator) {
        LOG_DEBUG << "#";

        QVector<QChar> separators;
        separators << QChar(',');
        if (spaceIsSeparator) { separators << QChar::Space; }
        QStringList keywords;
        Helpers::splitKeywords(rawKeywords.trimmed(), separators, keywords);

        doSetKeywords(keywords);
    }

    bool ArtworkProxyBase::getHasTitleWordSpellError(const QString &word) {
        auto *keywordsModel = getBasicMetadataModel();
        return keywordsModel->hasTitleWordSpellError(word);
    }

    bool ArtworkProxyBase::getHasDescriptionWordSpellError(const QString &word) {
        auto *keywordsModel = getBasicMetadataModel();
        return keywordsModel->hasDescriptionWordSpellError(word);
    }

    bool ArtworkProxyBase::getHasTitleSpellErrors() {
        auto *keywordsModel = getBasicMetadataModel();
        return keywordsModel->hasTitleSpellError();
    }

    bool ArtworkProxyBase::getHasDescriptionSpellError() {
        auto *keywordsModel = getBasicMetadataModel();
        return keywordsModel->hasDescriptionSpellError();
    }

    bool ArtworkProxyBase::getHasKeywordsSpellError() {
        auto *keywordsModel = getBasicMetadataModel();
        return keywordsModel->hasKeywordsSpellError();
    }

    bool ArtworkProxyBase::doExpandPreset(int keywordIndex,
                                          KeywordsPresets::ID_t presetID,
                                          KeywordsPresets::IPresetsManager &presetsManager) {
        bool success = false;
        LOG_INFO << "keyword" << keywordIndex << "preset" << presetID;
        QStringList keywords;

        if (presetsManager.tryGetPreset(presetID, keywords)) {
            auto *metadataOperator = getMetadataOperator();
            if (metadataOperator->expandPreset(keywordIndex, keywords)) {
                signalKeywordsCountChanged();
                spellCheckKeywords();
                doJustEdited();
                success = true;
            }
        }

        return success;
    }

    bool ArtworkProxyBase::doAppendPreset(KeywordsPresets::ID_t presetID,
                                          KeywordsPresets::IPresetsManager &presetsManager) {
        bool success = false;
        LOG_INFO << "preset ID" << presetID;
        QStringList keywords;

        if (presetsManager.tryGetPreset(presetID, keywords)) {
            auto *metadataOperator = getMetadataOperator();
            if (metadataOperator->appendPreset(keywords)) {
                signalKeywordsCountChanged();
                spellCheckKeywords();
                doJustEdited();
                success = true;
            }
        }

        return success;
    }

    bool ArtworkProxyBase::doExpandLastKeywordAsPreset(KeywordsPresets::IPresetsManager &presetsManager) {
        LOG_DEBUG << "#";
        bool success = false;

        auto *keywordsModel = getBasicMetadataModel();
        int keywordIndex = keywordsModel->getKeywordsCount() - 1;
        QString lastKeyword = keywordsModel->retrieveKeyword(keywordIndex);
        KeywordsPresets::ID_t presetID = 0;

        if (presetsManager.tryFindSinglePresetByName(lastKeyword, false, presetID)) {
            success = doExpandPreset(keywordIndex, presetID, presetsManager);
        } else {
            LOG_DEBUG << "Preset not found";
        }

        return success;
    }

    bool ArtworkProxyBase::doRemovePreset(KeywordsPresets::ID_t presetID, KeywordsPresets::IPresetsManager &presetsManager) {
        LOG_INFO << presetID;
        bool success = false;
        QStringList keywords;

        if (presetsManager.tryGetPreset(presetID, keywords)) {
            for (auto &keyword: keywords) {
                keyword = keyword.toLower();
            }

            success = doRemoveKeywords(keywords.toSet(), false);
        }

        return success;
    }

    void ArtworkProxyBase::doHandleUserDictChanged(const QStringList &keywords, bool overwritten) {
        LOG_DEBUG << "#";
        Q_ASSERT(!keywords.isEmpty());

        auto *metadataModel = getBasicMetadataModel();

        SpellCheck::SpellCheckItemInfo *info = metadataModel->getSpellCheckInfo();

        if (!overwritten) {
            info->removeWordsFromErrors(keywords);
            // special case after words added to dict
            std::vector<Artworks::BasicKeywordsModel *> items;
            items.push_back(metadataModel);
        }
        else {
            info->clear();
        }

        submitForInspection();
    }

    void ArtworkProxyBase::doHandleUserDictCleared() {
        LOG_DEBUG << "#";
        submitForInspection();
    }

    void ArtworkProxyBase::doCopyToQuickBuffer() {
        LOG_DEBUG << "#";
        auto *metadataModel = getBasicMetadataModel();
        auto *quickBuffer = m_CommandManager->getQuickBuffer();
        quickBuffer->setFromBasicModel(metadataModel);
    }

    bool ArtworkProxyBase::hasKeywords(const QStringList &keywordsList) {
        auto *metadataOperator = getMetadataOperator();
        bool result = metadataOperator->hasKeywords(keywordsList);
        return result;
    }

    void ArtworkProxyBase::doJustEdited() {
        /*metadataOperator->justEdited();*/
    }

    bool ArtworkProxyBase::doAcceptCompletionAsPreset(int completionID,
                                                      AutoComplete::ICompletionSource &completionSource,
                                                      KeywordsPresets::IPresetsManager &presetsManager) {
        bool accepted = false;

        auto completionItem = completionSource.getAcceptedCompletion(completionID);
        if (!completionItem) {
            LOG_WARNING << "Completion is not available anymore";
            return false;
        }

        const int presetID = completionItem->getPresetID();

        if (completionItem->isPreset() || (completionItem->canBePreset() && completionItem->shouldExpandPreset())) {
            accepted = doAppendPreset(presetID, presetsManager);
        }/* --------- this is handled in the edit field -----------
            else if (completionItem->isKeyword()) {
            doAppendKeyword(completionItem->getCompletion());
            accepted = true;
        }*/

        return accepted;
    }
}

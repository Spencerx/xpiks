/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2017 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "combinedartworksmodel.h"
#include "../Helpers/indiceshelper.h"
#include "../Commands/combinededitcommand.h"
#include "../Commands/commandmanager.h"
#include "../Commands/commandbase.h"
#include "../Suggestion/keywordssuggestor.h"
#include "artworkmetadata.h"
#include "artworkelement.h"
#include "../SpellCheck/spellcheckiteminfo.h"
#include "../Common/defines.h"
#include "../QMLExtensions/colorsmodel.h"

#define MAX_EDITING_PAUSE_RESTARTS 12

namespace Models {
    CombinedArtworksModel::CombinedArtworksModel(QObject *parent):
        ArtworksViewModel(parent),
        ArtworkProxyBase(),
        Common::DelayedActionEntity(1000, MAX_EDITING_PAUSE_RESTARTS),
        m_CommonKeywordsModel(m_HoldPlaceholder, this),
        m_EditFlags(Common::CombinedEditFlags::None),
        m_ModifiedFlags(0)
    {
        m_CommonKeywordsModel.setSpellCheckInfo(&m_SpellCheckInfo);

        QObject::connect(&m_CommonKeywordsModel, &Common::BasicMetadataModel::titleSpellingChanged,
                         this, &CombinedArtworksModel::onTitleSpellingChanged);
        QObject::connect(&m_CommonKeywordsModel, &Common::BasicMetadataModel::descriptionSpellingChanged,
                         this, &CombinedArtworksModel::onDescriptionSpellingChanged);

        QObject::connect(&m_CommonKeywordsModel, &Common::BasicMetadataModel::completionsAvailable,
                         this, &CombinedArtworksModel::completionsAvailable);

        QObject::connect(&m_CommonKeywordsModel, &Common::BasicMetadataModel::afterSpellingErrorsFixed,
                         this, &CombinedArtworksModel::spellCheckErrorsFixedHandler);

        QObject::connect(this, &CombinedArtworksModel::editingPaused,
                         this, &CombinedArtworksModel::onEditingPaused);

        QObject::connect(&m_CommonKeywordsModel, &Common::BasicMetadataModel::descriptionSpellingChanged,
                         this, &CombinedArtworksModel::descriptionSpellingChanged);
        QObject::connect(&m_CommonKeywordsModel, &Common::BasicMetadataModel::titleSpellingChanged,
                         this, &CombinedArtworksModel::titleSpellingChanged);
        QObject::connect(&m_CommonKeywordsModel, &Common::BasicMetadataModel::keywordsSpellingChanged,
                         this, &CombinedArtworksModel::keywordsSpellingChanged);
    }

    void CombinedArtworksModel::setArtworks(MetadataIO::WeakArtworksSnapshot &artworks) {
        ArtworksViewModel::setArtworks(artworks);

        recombineArtworks();

        if (getArtworksCount() == 1) {
            enableAllFields();
        }
    }

    void CombinedArtworksModel::recombineArtworks() {
        LOG_INFO << getArtworksCount() << "artwork(s)";

        LOG_DEBUG << "Before recombine description:" << getDescription();
        LOG_DEBUG << "Before recombine title:" << getTitle();
        LOG_DEBUG << "Before recombine keywords:" << getKeywordsString();

        if (isEmpty()) {
            return;
        }

        if (getArtworksCount() == 1) {
            assignFromOneArtwork();
        } else {
            assignFromManyArtworks();
        }

        LOG_DEBUG << "After recombine description:" << getDescription();
        LOG_DEBUG << "After recombine title:" << getTitle();
        LOG_DEBUG << "After recombine keywords:" << getKeywordsString();

        xpiks()->submitItemForSpellCheck(&m_CommonKeywordsModel);
    }

    void CombinedArtworksModel::acceptSuggestedKeywords(const QStringList &keywords) {
        LOG_INFO << keywords.size() << "keyword(s)";
        foreach(const QString &keyword, keywords) {
            this->appendKeyword(keyword);
        }
    }

    void CombinedArtworksModel::setDescription(const QString &value) {
        if (doSetDescription(value)) {
            signalDescriptionChanged();
            setDescriptionModified(true);
        }
    }

    void CombinedArtworksModel::setTitle(const QString &value) {
        if (doSetTitle(value)) {
            signalTitleChanged();
            setTitleModified(true);
        }
    }

    void CombinedArtworksModel::setKeywords(const QStringList &keywords) {
        ArtworkProxyBase::setKeywords(keywords);
        setKeywordsModified(true);
    }

    void CombinedArtworksModel::setChangeDescription(bool value) {
        LOG_INFO << value;
        auto flag = Common::CombinedEditFlags::EditDescription;
        if (Common::HasFlag(m_EditFlags, flag) != value) {
            Common::ApplyFlag(m_EditFlags, value, flag);
            emit changeDescriptionChanged();
        }
    }

    void CombinedArtworksModel::setChangeTitle(bool value) {
        LOG_INFO << value;
        auto flag = Common::CombinedEditFlags::EditTitle;
        if (Common::HasFlag(m_EditFlags, flag) != value) {
            Common::ApplyFlag(m_EditFlags, value, flag);
            emit changeTitleChanged();
        }
    }

    void CombinedArtworksModel::setChangeKeywords(bool value) {
        LOG_INFO << value;
        auto flag = Common::CombinedEditFlags::EditKeywords;
        if (Common::HasFlag(m_EditFlags, flag) != value) {
            Common::ApplyFlag(m_EditFlags, value, flag);
            emit changeKeywordsChanged();
        }
    }

    void CombinedArtworksModel::setAppendKeywords(bool value) {
        LOG_INFO << value;
        auto flag = Common::CombinedEditFlags::AppendKeywords;
        if (Common::HasFlag(m_EditFlags, flag) != value) {
            Common::ApplyFlag(m_EditFlags, value, flag);
            emit appendKeywordsChanged();
        }
    }

#ifdef CORE_TESTS
    QStringList CombinedArtworksModel::getKeywords() {
        return m_CommonKeywordsModel.getKeywords();
    }

#endif

    void CombinedArtworksModel::editKeyword(int index, const QString &replacement) {
        if (doEditKeyword(index, replacement)) {
            setKeywordsModified(true);
        }
    }

    QString CombinedArtworksModel::removeKeywordAt(int keywordIndex) {
        QString keyword;
        if (doRemoveKeywordAt(keywordIndex, keyword)) {
            setKeywordsModified(true);
        }
        return keyword;
    }

    void CombinedArtworksModel::removeLastKeyword() {
        QString keyword;
        if (doRemoveLastKeyword(keyword)) {
            setKeywordsModified(true);
        }
    }

    bool CombinedArtworksModel::appendKeyword(const QString &keyword) {
        bool added = doAppendKeyword(keyword);
        if (added) {
            setKeywordsModified(true);
        }

        return added;
    }

    void CombinedArtworksModel::pasteKeywords(const QStringList &keywords) {
        if (doAppendKeywords(keywords) > 0) {
            setKeywordsModified(true);
        }
    }

    void CombinedArtworksModel::saveEdits() {
        LOG_INFO << "edit flags:" << (int)m_EditFlags << "modified flags:" << m_ModifiedFlags;
        bool needToSave = false;

        if (getChangeTitle() ||
            getChangeDescription() ||
            getChangeKeywords()) {
            needToSave = getArtworksCount() > 1;
            needToSave = needToSave || (getChangeKeywords() && areKeywordsModified());
            needToSave = needToSave || isSpellingFixed();
            needToSave = needToSave || (getChangeTitle() && isTitleModified());
            needToSave = needToSave || (getChangeDescription() && isDescriptionModified());
        }

        if (needToSave) {
            processCombinedEditCommand();
        } else {
            LOG_DEBUG << "nothing to save";
        }
    }

    void CombinedArtworksModel::clearKeywords() {
        if (doClearKeywords()) {
            setKeywordsModified(true);
        }
    }

    void CombinedArtworksModel::suggestCorrections() {
        doSuggestCorrections();
    }

    void CombinedArtworksModel::setupDuplicatesModel() {
        doSetupDuplicatesModel();
    }

    void CombinedArtworksModel::initDescriptionHighlighting(QQuickTextDocument *document) {
        SpellCheck::SpellCheckItemInfo *info = m_CommonKeywordsModel.getSpellCheckInfo();

        if (info == NULL) {
            Q_ASSERT(false);
            // OneItem edits will use artwork's spellcheckinfo
            // combined edit will use this one
            info = &m_SpellCheckInfo;
        }

        QMLExtensions::ColorsModel *colorsModel = m_CommandManager->getColorsModel();
        info->createHighlighterForDescription(document->textDocument(), colorsModel, &m_CommonKeywordsModel);
        m_CommonKeywordsModel.notifyDescriptionSpellingChanged();
    }

    void CombinedArtworksModel::initTitleHighlighting(QQuickTextDocument *document) {
        SpellCheck::SpellCheckItemInfo *info = m_CommonKeywordsModel.getSpellCheckInfo();

        if (info == NULL) {
            Q_ASSERT(false);
            // OneItem edits will use artwork's spellcheckinfo
            // combined edit will use this one
            info = &m_SpellCheckInfo;
        }

        QMLExtensions::ColorsModel *colorsModel = m_CommandManager->getColorsModel();
        info->createHighlighterForTitle(document->textDocument(), colorsModel, &m_CommonKeywordsModel);
        m_CommonKeywordsModel.notifyTitleSpellingChanged();
    }

    void CombinedArtworksModel::assignFromSelected() {
        LOG_DEBUG << "#";
        recombineArtworks([](const ArtworkElement *item) { return item->getIsSelected(); });

        LOG_DEBUG << "After recombine description:" << getDescription();
        LOG_DEBUG << "After recombine title:" << getTitle();
        LOG_DEBUG << "After recombine keywords:" << getKeywordsString();

        xpiks()->submitItemForSpellCheck(&m_CommonKeywordsModel);
    }

    void CombinedArtworksModel::plainTextEdit(const QString &rawKeywords, bool spaceIsSeparator) {
        doPlainTextEdit(rawKeywords, spaceIsSeparator);
        setKeywordsModified(true);
    }

    bool CombinedArtworksModel::hasTitleWordSpellError(const QString &word) {
        return getHasTitleWordSpellError(word);
    }

    bool CombinedArtworksModel::hasDescriptionWordSpellError(const QString &word) {
        return getHasDescriptionWordSpellError(word);
    }

    void CombinedArtworksModel::expandPreset(int keywordIndex, unsigned int presetID) {
        if (doExpandPreset(keywordIndex, (KeywordsPresets::ID_t)presetID)) {
            setKeywordsModified(true);
        }
    }

    void CombinedArtworksModel::expandLastKeywordAsPreset() {
        doExpandLastKeywordAsPreset();
    }

    void CombinedArtworksModel::addPreset(unsigned int presetID) {
        if (doAddPreset((KeywordsPresets::ID_t)presetID)) {
            setKeywordsModified(true);
        }
    }

    void CombinedArtworksModel::initSuggestion() {
        doInitSuggestion();
    }

    void CombinedArtworksModel::registerAsCurrentItem() {
        LOG_DEBUG << "#";
        doRegisterAsCurrentItem();
    }

    void CombinedArtworksModel::copyToQuickBuffer() {
        LOG_DEBUG << "#";
        doCopyToQuickBuffer();
    }

    void CombinedArtworksModel::generateCompletions(const QString &prefix) {
        LOG_DEBUG << prefix;
        doGenerateCompletions(prefix);
    }

    bool CombinedArtworksModel::acceptCompletionAsPreset(int completionID) {
        LOG_DEBUG << completionID;
        return doAcceptCompletionAsPreset(completionID);
    }

    void CombinedArtworksModel::processCombinedEditCommand() {
        MetadataIO::ArtworksSnapshot::Container rawSnapshot(getRawSnapshot());

        std::shared_ptr<Commands::CombinedEditCommand> combinedEditCommand(new Commands::CombinedEditCommand(
                m_EditFlags,
                rawSnapshot,
                m_CommonKeywordsModel.getDescription(),
                m_CommonKeywordsModel.getTitle(),
                m_CommonKeywordsModel.getKeywords()));

        m_CommandManager->processCommand(combinedEditCommand);
    }

    void CombinedArtworksModel::enableAllFields() {
        setChangeDescription(true);
        setChangeTitle(true);
        setChangeKeywords(true);
    }

    void CombinedArtworksModel::assignFromOneArtwork() {
        LOG_DEBUG << "#";
        Q_ASSERT(getArtworksCount() == 1);
        ArtworkMetadata *metadata = getArtworkMetadata(0);

        if (!isDescriptionModified()) {
            initDescription(metadata->getDescription());
        }

        if (!isTitleModified()) {
            initTitle(metadata->getTitle());
        }

        if (!areKeywordsModified()) {
            initKeywords(metadata->getKeywords());
        }
    }

    void CombinedArtworksModel::assignFromManyArtworks() {
        recombineArtworks([](const ArtworkElement *) { return true; });
    }

    void CombinedArtworksModel::recombineArtworks(std::function<bool (const ArtworkElement *)> pred) {
        LOG_DEBUG << "#";

        bool descriptionsDiffer = false;
        bool titleDiffer = false;
        bool anyDifferent = false;
        QString description, title;
        QSet<QString> commonKeywords, unitedKeywords;
        QStringList firstItemKeywords;
        int firstItemKeywordsCount = 0;
        int firstNonEmptyIndex = 0;
        ArtworkMetadata *firstNonEmpty = nullptr;

        if (findNonEmptyData(pred, firstNonEmptyIndex, firstNonEmpty)) {
            description = firstNonEmpty->getDescription();
            title = firstNonEmpty->getTitle();
            firstItemKeywords = firstNonEmpty->getKeywords();
            // preserve case with List to Set convertion
            auto firstSet = firstItemKeywords.toSet();
            commonKeywords.unite(firstSet);
            firstItemKeywordsCount = firstSet.count();
        }

        processArtworks(pred,
                        [&](size_t index, ArtworkMetadata *metadata) {
            if (index == firstNonEmptyIndex) { return; }

            QString currDescription = metadata->getDescription();
            QString currTitle = metadata->getTitle();
            descriptionsDiffer = descriptionsDiffer || ((!currDescription.isEmpty()) && (description != currDescription));
            titleDiffer = titleDiffer || ((!currTitle.isEmpty()) && (title != currTitle));

            // preserve case with List to Set convertion
            auto currentSet = metadata->getKeywords().toSet();

            if (!currentSet.isEmpty()) {
                commonKeywords.intersect(currentSet);

                // used to detect if all items have same keywords
                if ((currentSet.count() == firstItemKeywordsCount) &&
                    (unitedKeywords.count() <= firstItemKeywordsCount)) {
                    unitedKeywords.unite(currentSet);
                } else {
                    anyDifferent = true;
                }
            }
        });

        if (!isEmpty()) {
            if (descriptionsDiffer) {
                description = "";
            }

            if (titleDiffer) {
                title = "";
            }

            initDescription(description);
            initTitle(title);

            if (!areKeywordsModified()) {
                if ((!anyDifferent) && unitedKeywords.subtract(commonKeywords).isEmpty()) {
                    // all keywords are the same
                    initKeywords(firstItemKeywords);
                } else {
                    initKeywords(commonKeywords.toList());
                }
            }
        }
    }

    bool CombinedArtworksModel::findNonEmptyData(std::function<bool (const ArtworkElement *)> pred, int &index,  ArtworkMetadata *&artworkMetadata) {
        bool found = false, foundOther = false;
        int nonEmptyKeywordsIndex = -1, nonEmptyOtherIndex = -1;
        ArtworkMetadata *nonEmptyKeywordsMetadata = nullptr;
        ArtworkMetadata *nonEmptyOtherMetadata = nullptr;

        processArtworksEx(pred,
                        [&](size_t index, ArtworkMetadata *metadata) -> bool {
            if (!metadata->areKeywordsEmpty()) {
                nonEmptyKeywordsIndex = (int)index;
                nonEmptyKeywordsMetadata = metadata;
                found = true;
            } else {
                if (!foundOther) {
                    if (!metadata->getDescription().trimmed().isEmpty() ||
                            !metadata->getTitle().trimmed().isEmpty()) {
                        nonEmptyOtherIndex = (int)index;
                        nonEmptyOtherMetadata = metadata;
                        foundOther = true;
                    }
                }
            }

            bool shouldContinue = !found;
            return shouldContinue;
        });

        if (found) {
            LOG_INFO << "Found artwork with non-empty keywords at" << nonEmptyKeywordsIndex;
            artworkMetadata = nonEmptyKeywordsMetadata;
            index = nonEmptyKeywordsIndex;
        } else if (foundOther) {
            LOG_INFO << "Found artwork with non-empty other data at" << nonEmptyOtherIndex;
            artworkMetadata = nonEmptyOtherMetadata;
            index = nonEmptyOtherIndex;
        } else {
            LOG_WARNING << "All artworks seem to be blank";
        }

        return found || foundOther;
    }

    void CombinedArtworksModel::onDescriptionSpellingChanged() {
        emit descriptionChanged();
    }

    void CombinedArtworksModel::onTitleSpellingChanged() {
        emit titleChanged();
    }

    void CombinedArtworksModel::spellCheckErrorsFixedHandler() {
        setSpellingFixed(true);
        // for possible squeezing after replace
        emit keywordsCountChanged();
    }

    void CombinedArtworksModel::generateAboutToBeRemoved() {
        LOG_DEBUG << "#";
        m_CommonKeywordsModel.notifyAboutToBeRemoved();
    }

    void CombinedArtworksModel::userDictUpdateHandler(const QStringList &keywords, bool overwritten) {
        LOG_DEBUG << "#";
        doHandleUserDictChanged(keywords, overwritten);
    }

    void CombinedArtworksModel::userDictClearedHandler() {
        LOG_DEBUG << "#";
        doHandleUserDictCleared();
    }

    void CombinedArtworksModel::onEditingPaused() {
        LOG_INTEGR_TESTS_OR_DEBUG << "#";
        xpiks()->submitItemForSpellCheck(&m_CommonKeywordsModel);
    }

    void CombinedArtworksModel::doJustEdited() {
        justChanged();
    }

    qint64 CombinedArtworksModel::getSpecialItemID() {
        return SPECIAL_ID_COMBINED_MODEL;
    }

    bool CombinedArtworksModel::doRemoveSelectedArtworks() {
        LOG_DEBUG << "#";
        bool anyRemoved = ArtworksViewModel::doRemoveSelectedArtworks();
        if (anyRemoved) {
            if (!isEmpty()) {
                recombineArtworks();
            }
        }

        return anyRemoved;
    }

    void CombinedArtworksModel::doResetModel() {
        LOG_DEBUG << "#";
        ArtworksViewModel::doResetModel();

        m_SpellCheckInfo.clear();

        // TEMPORARY (enable everything on initial launch) --
        m_ModifiedFlags = 0;
        m_EditFlags = Common::CombinedEditFlags::None;
        enableAllFields();
        // TEMPORARY (enable everything on initial launch) --

        initDescription("");
        initTitle("");
        initKeywords(QStringList());

        xpiks()->clearCurrentItem();
    }

    void CombinedArtworksModel::doOnTimer() {
        emit editingPaused();
    }

    bool CombinedArtworksModel::removeUnavailableItems() {
        LOG_DEBUG << "#";
        bool anyRemoved = ArtworksViewModel::removeUnavailableItems();
        if (anyRemoved) {
            if (!isEmpty()) {
                recombineArtworks();
            }
        }

        return anyRemoved;
    }
}

/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2017 Taras Kushnir <kushnirTV@gmail.com>
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

#include "combinedartworksmodel.h"
#include "../Helpers/indiceshelper.h"
#include "../Commands/combinededitcommand.h"
#include "../Commands/commandmanager.h"
#include "../Commands/commandbase.h"
#include "../Suggestion/keywordssuggestor.h"
#include "artworkmetadata.h"
#include "metadataelement.h"
#include "../SpellCheck/spellcheckiteminfo.h"
#include "../Common/defines.h"
#include "../QMLExtensions/colorsmodel.h"

namespace Models {
    CombinedArtworksModel::CombinedArtworksModel(QObject *parent):
        ArtworksViewModel(parent),
        ArtworkProxyBase(),
        m_CommonKeywordsModel(m_HoldPlaceholder, this),
        m_EditFlags(Common::CombinedEditFlags::None),
        m_ModifiedFlags(0) {
        m_CommonKeywordsModel.setSpellCheckInfo(&m_SpellCheckInfo);

        QObject::connect(&m_CommonKeywordsModel, SIGNAL(spellCheckErrorsChanged()),
                         this, SLOT(spellCheckErrorsChangedHandler()));

        QObject::connect(&m_CommonKeywordsModel, SIGNAL(completionsAvailable()),
                         this, SIGNAL(completionsAvailable()));

        QObject::connect(&m_CommonKeywordsModel, SIGNAL(afterSpellingErrorsFixed()),
                         this, SLOT(spellCheckErrorsFixedHandler()));
    }

    void CombinedArtworksModel::setArtworks(std::vector<MetadataElement> &artworks) {
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

        m_CommandManager->submitItemForSpellCheck(&m_CommonKeywordsModel);
    }

    void CombinedArtworksModel::acceptSuggestedKeywords(const QStringList &keywords) {
        LOG_INFO << keywords.size() << "keyword(s)";
        foreach(const QString &keyword, keywords) {
            this->appendKeyword(keyword);
        }
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

    void CombinedArtworksModel::appendKeyword(const QString &keyword) {
        if (doAppendKeyword(keyword)) {
            setKeywordsModified(true);
        }
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
        m_CommonKeywordsModel.notifyDescriptionSpellCheck();
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
        m_CommonKeywordsModel.notifyTitleSpellCheck();
    }

    void CombinedArtworksModel::spellCheckDescription() {
        doSpellCheckDescription();
    }

    void CombinedArtworksModel::spellCheckTitle() {
        doSpellCheckTitle();
    }

    void CombinedArtworksModel::assignFromSelected() {
        LOG_DEBUG << "#";
        recombineArtworks([](const MetadataElement &item) { return item.isSelected(); });

        LOG_DEBUG << "After recombine description:" << getDescription();
        LOG_DEBUG << "After recombine title:" << getTitle();
        LOG_DEBUG << "After recombine keywords:" << getKeywordsString();

        m_CommandManager->submitItemForSpellCheck(&m_CommonKeywordsModel);
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

    void CombinedArtworksModel::expandPreset(int keywordIndex, int presetIndex) {
        if (doExpandPreset(keywordIndex, presetIndex)) {
            setKeywordsModified(true);
        }
    }

    void CombinedArtworksModel::expandLastKeywordAsPreset() {
        doExpandLastKeywordAsPreset();
    }

    void CombinedArtworksModel::addPreset(int presetIndex) {
        if (doAddPreset(presetIndex)) {
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

    void CombinedArtworksModel::processCombinedEditCommand() {
        auto &artworksList = getArtworksList();

        std::vector<Models::MetadataElement> artworksToProcess(artworksList);

        std::shared_ptr<Commands::CombinedEditCommand> combinedEditCommand(new Commands::CombinedEditCommand(
                m_EditFlags,
                artworksToProcess,
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
        recombineArtworks([](const MetadataElement &) { return true; });
    }

    void CombinedArtworksModel::recombineArtworks(std::function<bool (const MetadataElement &)> pred) {
        LOG_DEBUG << "#";

        bool anyItemsProcessed = false;
        bool descriptionsDiffer = false;
        bool titleDiffer = false;
        bool anyDifferent = false;
        QString description, title;
        QSet<QString> commonKeywords, unitedKeywords;
        QStringList firstItemKeywords;
        int firstItemKeywordsCount = 0;

        processArtworks(pred,
                        [&](int, ArtworkMetadata *metadata) {
            if (!anyItemsProcessed) {
                description = metadata->getDescription();
                title = metadata->getTitle();
                firstItemKeywords = metadata->getKeywords();
                // preserve case with List to Set convertion
                auto firstSet = firstItemKeywords.toSet();
                commonKeywords.unite(firstSet);
                firstItemKeywordsCount = firstSet.count();
                anyItemsProcessed = true;
                return;
            }

            QString currDescription = metadata->getDescription();
            QString currTitle = metadata->getTitle();
            descriptionsDiffer = descriptionsDiffer || description != currDescription;
            titleDiffer = titleDiffer || title != currTitle;
            // preserve case with List to Set convertion
            auto currentSet = metadata->getKeywords().toSet();
            commonKeywords.intersect(currentSet);

            // used to detect if all items have same keywords
            if ((currentSet.count() == firstItemKeywordsCount) &&
                (unitedKeywords.count() <= firstItemKeywordsCount)) {
                unitedKeywords.unite(currentSet);
            } else {
                anyDifferent = true;
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

    void CombinedArtworksModel::spellCheckErrorsChangedHandler() {
        emit descriptionChanged();
        emit titleChanged();
    }

    void CombinedArtworksModel::spellCheckErrorsFixedHandler() {
        setSpellingFixed(true);
        // for possible squeezing after replace
        emit keywordsCountChanged();
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

        m_CommandManager->clearCurrentItem();
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
}

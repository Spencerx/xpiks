/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "combinedartworksmodel.h"

#include <cstddef>
#include <initializer_list>
#include <utility>
#include <vector>

#include <QQmlEngine>
#include <QSet>
#include <QtDebug>
#include <QtGlobal>

#include "Artworks/artworkelement.h"
#include "Artworks/artworkmetadata.h"
#include "Artworks/basickeywordsmodel.h"
#include "Artworks/basicmetadatamodel.h"
#include "Artworks/basicmodelsource.h"
#include "Commands/Base/compositecommandtemplate.h"
#include "Commands/Base/icommandtemplate.h"
#include "Commands/Base/templatedcommand.h"
#include "Commands/Editing/clearactionmodeltemplate.h"
#include "Commands/Editing/editartworkstemplate.h"
#include "Commands/Editing/modifyartworkscommand.h"
#include "Commands/artworksupdatetemplate.h"
#include "Common/defines.h"
#include "Common/delayedactionentity.h"
#include "Common/flags.h"
#include "Common/logging.h"
#include "KeywordsPresets/presetmodel.h"
#include "Models/Artworks/artworksviewmodel.h"
#include "Models/Editing/artworkproxybase.h"
#include "Models/Editing/currenteditableproxyartwork.h"
#include "Models/Editing/quickbuffermessage.h"
#include "Services/SpellCheck/spellcheckinfo.h"

#define MAX_EDITING_PAUSE_RESTARTS 12

namespace Models {
    CombinedArtworksModel::CombinedArtworksModel(Services::IArtworksUpdater &artworksUpdater,
                                                 KeywordsPresets::IPresetsManager &presetsManager,
                                                 QObject *parent):
        ArtworksViewModel(parent),
        ArtworkProxyBase(),
        Common::DelayedActionEntity(1000, MAX_EDITING_PAUSE_RESTARTS),
        m_PresetsManager(presetsManager),
        m_ArtworksUpdater(artworksUpdater),
        m_CommonKeywordsModel(m_SpellCheckInfo, this),
        m_EditFlags(Common::ArtworkEditFlags::None),
        m_ModifiedFlags(0)
    {
        QObject::connect(&m_CommonKeywordsModel, &Artworks::BasicMetadataModel::titleSpellingChanged,
                         this, &CombinedArtworksModel::onTitleSpellingChanged);
        QObject::connect(&m_CommonKeywordsModel, &Artworks::BasicMetadataModel::descriptionSpellingChanged,
                         this, &CombinedArtworksModel::onDescriptionSpellingChanged);

        QObject::connect(&m_CommonKeywordsModel, &Artworks::BasicMetadataModel::completionsAvailable,
                         this, &CombinedArtworksModel::completionsAvailable);

        QObject::connect(&m_CommonKeywordsModel, &Artworks::BasicMetadataModel::afterSpellingErrorsFixed,
                         this, &CombinedArtworksModel::spellCheckErrorsFixedHandler);

        QObject::connect(this, &CombinedArtworksModel::editingPaused,
                         this, &CombinedArtworksModel::onEditingPaused);

        QObject::connect(&m_CommonKeywordsModel, &Artworks::BasicMetadataModel::descriptionSpellingChanged,
                         this, &CombinedArtworksModel::descriptionSpellingChanged);
        QObject::connect(&m_CommonKeywordsModel, &Artworks::BasicMetadataModel::titleSpellingChanged,
                         this, &CombinedArtworksModel::titleSpellingChanged);
        QObject::connect(&m_CommonKeywordsModel, &Artworks::BasicMetadataModel::keywordsSpellingChanged,
                         this, &CombinedArtworksModel::keywordsSpellingChanged);
    }

    void CombinedArtworksModel::setArtworks(Artworks::ArtworksSnapshot const &artworks) {
        ArtworksViewModel::setArtworks(artworks);

        recombineArtworks();

        registerAsCurrentEditable();
    }

    void CombinedArtworksModel::initKeywords(const QStringList &keywords, bool onlyAppend) {
         m_CommonKeywordsModel.setKeywords(keywords);
         setKeywordsModified(false);
         setChangeKeywords(true);
         setAppendKeywords(onlyAppend);
    }

    void CombinedArtworksModel::initDescription(const QString &description, bool enabled) {
        setDescription(description);
        setDescriptionModified(false);
        setChangeDescription(enabled);
    }

    void CombinedArtworksModel::initTitle(const QString &title, bool enabled) {
         setTitle(title);
         setTitleModified(false);
         setChangeTitle(enabled);
    }

    void CombinedArtworksModel::recombineArtworks() {
        LOG_INFO << getArtworksCount() << "artwork(s)";

        LOG_DEBUG << "Before recombine description:" << getDescription();
        LOG_DEBUG << "Before recombine title:" << getTitle();
        LOG_DEBUG << "Before recombine keywords:" << getKeywordsString();

        if (isEmpty()) { return; }

        if (getArtworksCount() == 1) {
            assignFromOneArtwork();
        } else {
            assignFromManyArtworks();
        }

        LOG_DEBUG << "After recombine description:" << getDescription();
        LOG_DEBUG << "After recombine title:" << getTitle();
        LOG_DEBUG << "After recombine keywords:" << getKeywordsString();

        submitForInspection();
    }

    void CombinedArtworksModel::registerAsCurrentEditable() {
        LOG_DEBUG << "#";
        sendMessage(std::make_shared<CurrentEditableProxyArtwork>(*this));
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
        LOG_DEBUG << keywords.size() << "keywords";
        ArtworkProxyBase::setKeywords(keywords);
        setKeywordsModified(true);
    }

    void CombinedArtworksModel::setChangeDescription(bool value) {
        LOG_INFO << value;
        auto flag = Common::ArtworkEditFlags::EditDescription;
        if (Common::HasFlag(m_EditFlags, flag) != value) {
            Common::ApplyFlag(m_EditFlags, value, flag);
            emit changeDescriptionChanged();
        }
    }

    void CombinedArtworksModel::setChangeTitle(bool value) {
        LOG_INFO << value;
        auto flag = Common::ArtworkEditFlags::EditTitle;
        if (Common::HasFlag(m_EditFlags, flag) != value) {
            Common::ApplyFlag(m_EditFlags, value, flag);
            emit changeTitleChanged();
        }
    }

    void CombinedArtworksModel::setChangeKeywords(bool value) {
        LOG_INFO << value;
        auto flag = Common::ArtworkEditFlags::EditKeywords;
        if (Common::HasFlag(m_EditFlags, flag) != value) {
            Common::ApplyFlag(m_EditFlags, value, flag);
            emit changeKeywordsChanged();
        }
    }

    void CombinedArtworksModel::setAppendKeywords(bool value) {
        LOG_INFO << value;
        auto flag = Common::ArtworkEditFlags::AppendKeywords;
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

    bool CombinedArtworksModel::moveKeyword(int from, int to) {
        return doMoveKeyword(from, to);
    }

    bool CombinedArtworksModel::appendKeyword(const QString &keyword) {
        const bool added = doAppendKeyword(keyword);
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

    void CombinedArtworksModel::clearKeywords() {
        if (doClearKeywords()) {
            setKeywordsModified(true);
        }
    }

    QObject *CombinedArtworksModel::getBasicModelObject() {
        QObject *item = &m_CommonKeywordsModel;
        QQmlEngine::setObjectOwnership(item, QQmlEngine::CppOwnership);
        return item;
    }

    void CombinedArtworksModel::assignFromSelected() {
        LOG_DEBUG << "#";
        recombineArtworks([](std::shared_ptr<Artworks::ArtworkElement> const &item) { return item->getIsSelected(); });

        LOG_DEBUG << "After recombine description:" << getDescription();
        LOG_DEBUG << "After recombine title:" << getTitle();
        LOG_DEBUG << "After recombine keywords:" << getKeywordsString();

        submitForInspection();
    }

    bool CombinedArtworksModel::hasTitleWordSpellError(const QString &word) {
        return getHasTitleWordSpellError(word);
    }

    bool CombinedArtworksModel::hasDescriptionWordSpellError(const QString &word) {
        return getHasDescriptionWordSpellError(word);
    }

    void CombinedArtworksModel::expandPreset(int keywordIndex, unsigned int presetID) {
        if (doExpandPreset(keywordIndex, (KeywordsPresets::ID_t)presetID, m_PresetsManager)) {
            setKeywordsModified(true);
        }
    }

    void CombinedArtworksModel::expandLastKeywordAsPreset() {
        doExpandLastKeywordAsPreset(m_PresetsManager);
    }

    void CombinedArtworksModel::addPreset(unsigned int presetID) {
        if (doAppendPreset((KeywordsPresets::ID_t)presetID, m_PresetsManager)) {
            setKeywordsModified(true);
        }
    }

#ifdef UI_TESTS
    void CombinedArtworksModel::clearModel() {
        LOG_DEBUG << "#";
        m_CommonKeywordsModel.clearModel();
        setChangeTitle(true);
        setChangeDescription(true);
        setChangeKeywords(true);
        emit descriptionChanged();
        emit titleChanged();
        emit keywordsCountChanged();
    }
#endif

    std::shared_ptr<Commands::ICommand> CombinedArtworksModel::getActionCommand(bool yesno) {
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

        if (needToSave && yesno) {
            using namespace Commands;
            using ArtworksTemplate = Commands::ICommandTemplate<Artworks::ArtworksSnapshot>;
            using ArtworksTemplateComposite = Commands::CompositeCommandTemplate<Artworks::ArtworksSnapshot>;
            auto snapshot = createSnapshot();
            return std::make_shared<ModifyArtworksCommand>(
                        std::move(snapshot),
                        std::make_shared<ArtworksTemplateComposite>(
                            std::initializer_list<std::shared_ptr<ArtworksTemplate>>{
                                std::make_shared<EditArtworksTemplate>(
                                m_CommonKeywordsModel.getTitle(),
                                m_CommonKeywordsModel.getDescription(),
                                m_CommonKeywordsModel.getKeywords(),
                                m_EditFlags),
                                std::make_shared<Commands::ClearActionModelTemplate>(*this),
                                std::make_shared<Commands::ArtworksSnapshotUpdateTemplate>(m_ArtworksUpdater)}));
        } else {
            LOG_DEBUG << "nothing to save";
            using TemplatedSnapshotCommand = Commands::TemplatedCommand<Artworks::ArtworksSnapshot>;
            return std::make_shared<TemplatedSnapshotCommand>(
                        Artworks::ArtworksSnapshot(),
                        std::make_shared<Commands::ClearActionModelTemplate>(*this));
        }
    }

    void CombinedArtworksModel::resetModel() {
        LOG_DEBUG << "#";
        ArtworksViewModel::resetModel();

        m_SpellCheckInfo.clear();

        // TEMPORARY (enable everything on initial launch) --
        m_ModifiedFlags = 0;
        m_EditFlags = Common::ArtworkEditFlags::None;

        const bool enable = true;
        initDescription("", enable);
        initTitle("", enable);
        initKeywords(QStringList(), false /*only append*/);
        // TEMPORARY (enable everything on initial launch) --

        // clear current editable
        sendMessage(std::shared_ptr<ICurrentEditable>());
    }

    void CombinedArtworksModel::copyToQuickBuffer() {
        LOG_DEBUG << "#";
        sendMessage(
                    QuickBufferMessage(
                        m_CommonKeywordsModel.getTitle(),
                        m_CommonKeywordsModel.getDescription(),
                        m_CommonKeywordsModel.getKeywords(),
                        false));
    }

    bool CombinedArtworksModel::acceptCompletionAsPreset(AutoComplete::ICompletionSource &completionSource, int completionID) {
        LOG_DEBUG << completionID;
        const bool accepted = doAcceptCompletionAsPreset(completionID, completionSource, m_PresetsManager);
        return accepted;
    }

    void CombinedArtworksModel::assignFromOneArtwork() {
        LOG_DEBUG << "#";
        Q_ASSERT(getArtworksCount() == 1);
        auto &artwork = accessItem(0)->getArtwork();
        const bool enable = true;

        if (!isDescriptionModified()) {
            initDescription(artwork->getDescription(), enable);
        }

        if (!isTitleModified()) {
            initTitle(artwork->getTitle(), enable);
        }

        if (!areKeywordsModified()) {
            initKeywords(artwork->getKeywords(), false /*only append*/);
        }
    }

    void CombinedArtworksModel::assignFromManyArtworks() {
        LOG_DEBUG << "#";
        recombineArtworks([](std::shared_ptr<Artworks::ArtworkElement> const &) { return true; });
    }

    void CombinedArtworksModel::recombineArtworks(std::function<bool (std::shared_ptr<Artworks::ArtworkElement> const &)> pred) {
        LOG_DEBUG << "#";

        bool descriptionsDiffer = false;
        bool titleDiffer = false;
        bool anyDifferent = false;
        QString description, title;
        QSet<QString> commonKeywords, unitedKeywords;
        QStringList firstItemKeywords;
        int firstItemKeywordsCount = 0;
        int firstNonEmptyIndex = 0;
        std::shared_ptr<Artworks::ArtworkMetadata> firstNonEmpty;

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
                        [&](size_t index, std::shared_ptr<Artworks::ArtworkMetadata> const &artwork) {
            if ((int)index == firstNonEmptyIndex) { return; }

            QString currDescription = artwork->getDescription();
            QString currTitle = artwork->getTitle();
            descriptionsDiffer = descriptionsDiffer || ((!currDescription.isEmpty()) && (description != currDescription));
            titleDiffer = titleDiffer || ((!currTitle.isEmpty()) && (title != currTitle));

            // preserve case with List to Set convertion
            auto currentSet = artwork->getKeywords().toSet();

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

            initDescription(description, !descriptionsDiffer);
            initTitle(title, !titleDiffer);

            if (!areKeywordsModified()) {
                if ((!anyDifferent) && unitedKeywords.subtract(commonKeywords).isEmpty()) {
                    // all keywords are the same
                    initKeywords(firstItemKeywords, false /*only append*/);
                } else {
                    initKeywords(commonKeywords.toList(), true /*only append*/);
                }
            }
        }
    }

    bool CombinedArtworksModel::findNonEmptyData(std::function<bool (std::shared_ptr<Artworks::ArtworkElement> const &)> pred,
                                                 int &index,
                                                 std::shared_ptr<Artworks::ArtworkMetadata> &artworkMetadata) {
        bool found = false, foundOther = false;
        int nonEmptyKeywordsIndex = -1, nonEmptyOtherIndex = -1;
        std::shared_ptr<Artworks::ArtworkMetadata> nonEmptyKeywordsArtwork;
        std::shared_ptr<Artworks::ArtworkMetadata> nonEmptyOtherMetadata;

        processArtworksEx(pred,
                        [&](size_t index, std::shared_ptr<Artworks::ArtworkMetadata> const &artwork) -> bool {
            if (!artwork->areKeywordsEmpty()) {
                nonEmptyKeywordsIndex = (int)index;
                nonEmptyKeywordsArtwork = artwork;
                found = true;
            } else {
                if (!foundOther) {
                    if (!artwork->getDescription().trimmed().isEmpty() ||
                            !artwork->getTitle().trimmed().isEmpty()) {
                        nonEmptyOtherIndex = (int)index;
                        nonEmptyOtherMetadata = artwork;
                        foundOther = true;
                    }
                }
            }

            bool shouldContinue = !found;
            return shouldContinue;
        });

        if (found) {
            LOG_INFO << "Found artwork with non-empty keywords at" << nonEmptyKeywordsIndex;
            artworkMetadata = nonEmptyKeywordsArtwork;
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

    void CombinedArtworksModel::userDictUpdateHandler(const QStringList &keywords, bool overwritten) {
        LOG_DEBUG << "#";
        doHandleUserDictChanged(keywords, overwritten);
    }

    void CombinedArtworksModel::userDictClearedHandler() {
        LOG_DEBUG << "#";
        doHandleUserDictCleared();
    }

    void CombinedArtworksModel::onEditingPaused() {
        LOG_VERBOSE_OR_DEBUG << "#";
        submitForInspection();
    }

    void CombinedArtworksModel::doJustEdited() {
        justChanged();
    }

    void CombinedArtworksModel::submitForInspection() {
        sendMessage(
                    BasicSpellCheckMessageType(
                        std::make_shared<Artworks::BasicModelSource>(m_CommonKeywordsModel)));
    }

    Common::ID_t CombinedArtworksModel::getSpecialItemID() {
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

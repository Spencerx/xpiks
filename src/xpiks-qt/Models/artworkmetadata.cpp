/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "artworkmetadata.h"
#include <QReadLocker>
#include <QWriteLocker>
#include <QStringBuilder>
#include <QDir>
#include <QFileInfo>
#include "../Helpers/keywordshelpers.h"
#include "settingsmodel.h"
#include "../SpellCheck/spellsuggestionsitem.h"
#include "../SpellCheck/spellcheckitem.h"
#include "../SpellCheck/spellcheckiteminfo.h"
#include "../Common/defines.h"
#include "../MetadataIO/cachedartwork.h"
#include "../MetadataIO/originalmetadata.h"

// twice average English word length
#define MAX_EDITING_PAUSE_RESTARTS 12
#define ARTWORK_EDITING_PAUSE 1000

#ifdef SIZE_MAX
  #define INVALID_INDEX SIZE_MAX
#else
  #define INVALID_INDEX (~(size_t)0)
#endif

namespace Models {
    ArtworkMetadata::ArtworkMetadata(const QString &filepath, qint64 ID, qint64 directoryID):
        Common::DelayedActionEntity(ARTWORK_EDITING_PAUSE, MAX_EDITING_PAUSE_RESTARTS),
        m_MetadataModel(m_Hold),
        m_FileSize(0),
        m_ArtworkFilepath(filepath),
        m_ID(ID),
        m_DirectoryID(directoryID),
        m_MetadataFlags(0),
        m_LastKnownIndex(INVALID_INDEX),
        m_WarningsFlags(Common::WarningFlags::None)
    {
        m_MetadataModel.setSpellCheckInfo(&m_SpellCheckInfo);

        QObject::connect(&m_MetadataModel, &Common::BasicMetadataModel::spellingInfoUpdated, this, &ArtworkMetadata::spellingInfoUpdated);

        QFileInfo fi(filepath);
        setIsReadOnlyFlag(!fi.isWritable());
    }

    ArtworkMetadata::~ArtworkMetadata() {
#if defined(QT_DEBUG) && defined(INTEGRATION_TESTS)
        Q_ASSERT(m_Hold.get() == 0);
#endif
    }

    void ArtworkMetadata::prepareForReimport() {
        LOG_INTEGRATION_TESTS << "#" << m_ID;
        setIsReimportPendingFlag(true);
    }

    bool ArtworkMetadata::initFromOrigin(const MetadataIO::OriginalMetadata &originalMetadata, bool overwrite) {
        LOG_INTEGR_TESTS_OR_DEBUG << "#" << m_ID << originalMetadata.m_Title << originalMetadata.m_Description << originalMetadata.m_Keywords;
        bool anythingChanged = false;
        QMutexLocker initLocker(&m_InitMutex);
        Q_UNUSED(initLocker);

        Q_ASSERT(getIsReimportPendingFlag() || !getIsInitializedFlag());

        if (getIsReimportPendingFlag() || !getIsAlmostInitializedFlag() || overwrite) {
            anythingChanged = initFromOriginBeforeStorageUnsafe(originalMetadata);
        } else {
            anythingChanged = initFromOriginAfterStorageUnsafe(originalMetadata);
        }

        setIsInitializedFlag(true);
        setFileSize(originalMetadata.m_FileSize);

        anythingChanged = initFromOriginUnsafe(originalMetadata) || anythingChanged;
        return anythingChanged;
    }

    bool ArtworkMetadata::initFromStorage(const MetadataIO::CachedArtwork &cachedArtwork) {
        LOG_INTEGR_TESTS_OR_DEBUG << "#" << m_ID << cachedArtwork.m_Title <<
                                 cachedArtwork.m_Description << cachedArtwork.m_Keywords;
        bool anythingChanged = false;
        QMutexLocker initLocker(&m_InitMutex);
        Q_UNUSED(initLocker);

        // we should not even get here when reimporting because we ignore local storage
        Q_ASSERT(!getIsReimportPendingFlag());
        if (getIsReimportPendingFlag()) {
            Q_ASSERT(getIsAlmostInitializedFlag() || getIsInitializedFlag());
            return false;
        }

        Q_ASSERT(!getIsAlmostInitializedFlag());

        if (!getIsInitializedFlag()) {
            anythingChanged = initFromStorageBeforeOriginUnsafe(cachedArtwork);
        } else {
            anythingChanged = initFromStorageAfterOriginUnsafe(cachedArtwork);
        }

        setIsAlmostInitializedFlag(true);

        anythingChanged = initFromStorageUnsafe(cachedArtwork) || anythingChanged;
        return anythingChanged;
    }

    void ArtworkMetadata::initAsEmpty(const MetadataIO::OriginalMetadata &originalMetadata) {
        QMutexLocker initLocker(&m_InitMutex);
        Q_UNUSED(initLocker);

        m_MetadataModel.clearModel();
        setIsInitializedFlag(true);
        setIsModifiedFlag(false);

        setFileSize(originalMetadata.m_FileSize);

        initFromOriginUnsafe(originalMetadata);

        setIsReimportPendingFlag(false);
    }

    void ArtworkMetadata::initAsEmpty() {
        QMutexLocker initLocker(&m_InitMutex);
        Q_UNUSED(initLocker);

        m_MetadataModel.clearModel();
        setIsInitializedFlag(true);
        setIsModifiedFlag(false);
    }

    bool ArtworkMetadata::initFromOriginBeforeStorageUnsafe(const MetadataIO::OriginalMetadata &originalMetadata) {
        bool anythingChanged = false;

        anythingChanged = m_MetadataModel.setTitle(originalMetadata.m_Title) || anythingChanged;
        anythingChanged = m_MetadataModel.setDescription(originalMetadata.m_Description) || anythingChanged;
        m_MetadataModel.setKeywords(originalMetadata.m_Keywords);
        anythingChanged = anythingChanged || !originalMetadata.m_Keywords.isEmpty();
        Q_ASSERT(getIsReimportPendingFlag() || !getIsModifiedFlag());
        if (getIsReimportPendingFlag()) {
            setIsModifiedFlag(false);
        }

        return anythingChanged;
    }

    bool ArtworkMetadata::initFromOriginAfterStorageUnsafe(const MetadataIO::OriginalMetadata &originalMetadata) {
        Q_ASSERT(getIsAlmostInitializedFlag());
        bool shouldPreserveModified = false;
        bool anythingChanged = false;

        if (!originalMetadata.m_Title.trimmed().isEmpty()) {
            anythingChanged = m_MetadataModel.setTitle(originalMetadata.m_Title) || anythingChanged;
        } else {
            shouldPreserveModified = shouldPreserveModified || !m_MetadataModel.isTitleEmpty();
        }

        if (!originalMetadata.m_Description.trimmed().isEmpty()) {
            anythingChanged = m_MetadataModel.setDescription(originalMetadata.m_Description) || anythingChanged;
        } else {
            shouldPreserveModified = shouldPreserveModified || (!m_MetadataModel.isDescriptionEmpty());
        }

        if (!m_MetadataModel.containsKeywords(originalMetadata.m_Keywords)) {
            m_MetadataModel.setKeywords(originalMetadata.m_Keywords);
            anythingChanged = true;
        } else {
            const int existingCount = m_MetadataModel.getKeywordsCount();
            const int originCount = originalMetadata.m_Keywords.count();
            // which should mean that this artwork has been already imported from storage
            shouldPreserveModified = shouldPreserveModified || (existingCount > originCount);
        }

        setIsModifiedFlag(shouldPreserveModified);

        return anythingChanged;
    }

    bool ArtworkMetadata::initFromStorageBeforeOriginUnsafe(const MetadataIO::CachedArtwork &cachedArtwork) {
        bool anythingChanged = false;
        anythingChanged = m_MetadataModel.setTitle(cachedArtwork.m_Title) || anythingChanged;
        anythingChanged = m_MetadataModel.setDescription(cachedArtwork.m_Description) || anythingChanged;
        m_MetadataModel.setKeywords(cachedArtwork.m_Keywords);
        anythingChanged = anythingChanged || !cachedArtwork.m_Keywords.isEmpty();
        Q_ASSERT(getIsModifiedFlag() == false);
        return anythingChanged;
    }

    bool ArtworkMetadata::initFromStorageAfterOriginUnsafe(const MetadataIO::CachedArtwork &cachedArtwork) {
        bool anythingChanged = false;

        if (m_MetadataModel.isTitleEmpty() && !cachedArtwork.m_Title.trimmed().isEmpty()) {
            anythingChanged = this->setTitle(cachedArtwork.m_Title) || anythingChanged;
        }

        if (m_MetadataModel.isDescriptionEmpty() && !cachedArtwork.m_Description.trimmed().isEmpty()) {
            anythingChanged = this->setDescription(cachedArtwork.m_Description) || anythingChanged;
        }

        const size_t addedCount = this->appendKeywords(cachedArtwork.m_Keywords);
        if (addedCount > 0) {
            anythingChanged = true;
        }

        return anythingChanged;
    }

    QString ArtworkMetadata::getBaseFilename() const {
        QFileInfo fi(m_ArtworkFilepath);
        return fi.fileName();
    }

    bool ArtworkMetadata::isInDirectory(const QString &directoryAbsolutePath) const {
        bool isInDir = false;
        Q_ASSERT(directoryAbsolutePath == QDir(directoryAbsolutePath).absolutePath());

        if (m_ArtworkFilepath.startsWith(directoryAbsolutePath)) {
            QFileInfo fi(m_ArtworkFilepath);
            QString artworksDirectory = fi.absolutePath();

            isInDir = artworksDirectory == directoryAbsolutePath;
        }

        return isInDir;
    }

    bool ArtworkMetadata::hasDuplicates() {
        return m_MetadataModel.hasDuplicates();
    }

    void ArtworkMetadata::clearModel() {
        if (!getIsInitializedFlag()) {
            LOG_WARNING << "#" << m_ID << "attempt to clear not initialized artwork";
            return;
        }

        m_MetadataModel.clearModel();
        markModified();
        justChanged();
    }

    bool ArtworkMetadata::clearKeywords() {
        if (!getIsInitializedFlag()) {
            LOG_WARNING << "#" << m_ID << "attempt to clear keywords of not initialized artwork";
            return false;
        }

        bool result = m_MetadataModel.clearKeywords();
        if (result) {
            markModified();
            justChanged();
        }
        return result;
    }

    bool ArtworkMetadata::editKeyword(size_t index, const QString &replacement) {
        if (!getIsInitializedFlag()) {
            LOG_WARNING << "#" << m_ID << "attempt to edit keyword int not initialized artwork";
            return false;
        }

        bool result = m_MetadataModel.editKeyword(index, replacement);
        if (result) {
            markModified();
            justChanged();
        }
        return result;
    }

    bool ArtworkMetadata::replace(const QString &replaceWhat, const QString &replaceTo, Common::SearchFlags flags) {
        if (!getIsInitializedFlag()) {
            LOG_WARNING << "#" << m_ID << "attempt to replace strings in not initialized artwork";
            return false;
        }

        bool result = m_MetadataModel.replace(replaceWhat, replaceTo, flags);
        if (result) {
            markModified();
            justChanged();
        }
        return result;
    }

    bool ArtworkMetadata::moveKeyword(size_t from, size_t to) {
        if (!getIsInitializedFlag()) {
            LOG_WARNING << "#" << m_ID << "attempt to move keyword" << from << to;
            return false;
        }

        bool result = m_MetadataModel.moveKeyword(from, to);
        if (result) {
            markModified();
            justChanged();
        }
        return result;
    }

    bool ArtworkMetadata::setDescription(const QString &value)  {
        if (!getIsInitializedFlag()) {
            LOG_WARNING << "#" << m_ID << "attempt to set description to" << value << "for not initialized artwork";
            return false;
        }

        bool result = m_MetadataModel.setDescription(value);

        if (result) {
            markModified();
            justChanged();
        }

        return result;
    }

    bool ArtworkMetadata::setTitle(const QString &value) {
        if (!getIsInitializedFlag()) {
            LOG_WARNING << "#" << m_ID << "attempt to set title to" << value << "for not initialized artwork";
            return false;
        }

        bool result = m_MetadataModel.setTitle(value);

        if (result) {
            markModified();
            justChanged();
        }

        return result;
    }

    void ArtworkMetadata::setKeywords(const QStringList &keywords)  {
        if (!getIsInitializedFlag()) {
            LOG_WARNING << "#" << m_ID << "attempt to set keywords for not initialized artwork";
            return;
        }

        m_MetadataModel.setKeywords(keywords);
        markModified();
        justChanged();
    }

    bool ArtworkMetadata::setIsSelected(bool value) {
        bool result = getIsSelectedFlag() != value;
        if (result) {
            setIsSelectedFlag(value);
            emit selectedChanged(value);
        }

        return result;
    }

    bool ArtworkMetadata::removeKeywordAt(size_t index, QString &removed) {
        bool result = m_MetadataModel.removeKeywordAt(index, removed);
        if (result) {
            markModified();
            justChanged();
        }

        return result;
    }

    bool ArtworkMetadata::removeLastKeyword(QString &removed) {
        if (!getIsInitializedFlag()) {
            LOG_WARNING << "#" << m_ID << "attempt to remove last keyword from not initialized artwork";
            return false;
        }

        bool result = m_MetadataModel.removeLastKeyword(removed);
        if (result) {
            markModified();
            justChanged();
        }

        return result;
    }

    bool ArtworkMetadata::appendKeyword(const QString &keyword) {
        if (!getIsInitializedFlag()) {
            LOG_WARNING << "#" << m_ID << "attempt to append keyword to not initialized artwork";
            return false;
        }

        bool result = m_MetadataModel.appendKeyword(keyword);
        if (result) {
            markModified();
            justChanged();
        }
        return result;
    }

    size_t ArtworkMetadata::appendKeywords(const QStringList &keywordsList) {
        if (!getIsInitializedFlag()) {
            LOG_WARNING << "#" << m_ID << "attempt to append keywords to not initialized artwork";
            return false;
        }

        size_t result = m_MetadataModel.appendKeywords(keywordsList);
        LOG_INFO << "#" << m_ID << "Appended" << result << "keywords out of" << keywordsList.length();
        if (result > 0) {
            markModified();
            justChanged();
        }
        return result;
    }

    bool ArtworkMetadata::removeKeywords(const QSet<QString> &keywordsSet, bool caseSensitive) {
        if (!getIsInitializedFlag()) {
            LOG_WARNING << "#" << m_ID << "attempt to remove keywords int not initialized artwork";
            return false;
        }

        bool result = m_MetadataModel.removeKeywords(keywordsSet, caseSensitive);
        LOG_INFO << "#" << m_ID << "Removed keywords:" << result;
        if (result) {
            markModified();
            justChanged();
        }
        return result;
    }

    Common::KeywordReplaceResult ArtworkMetadata::fixKeywordSpelling(size_t index, const QString &existing, const QString &replacement) {
        if (!getIsInitializedFlag()) {
            LOG_WARNING << "#" << m_ID << "attempt to fix keyword for not initialized artwork";
            return Common::KeywordReplaceResult::Unknown;
        }

        auto result = m_MetadataModel.fixKeywordSpelling(index, existing, replacement);
        if (Common::KeywordReplaceResult::Succeeded == result) {
            markModified();
            justChanged();
        }

        return result;
    }

    bool ArtworkMetadata::fixDescriptionSpelling(const QString &word, const QString &replacement) {
        if (!getIsInitializedFlag()) {
            LOG_WARNING << "#" << m_ID << "attempt to fix description for not initialized artwork";
            return false;
        }

        bool result = m_MetadataModel.fixDescriptionSpelling(word, replacement);
        if (result) {
            markModified();
            justChanged();
        }

        return result;
    }

    bool ArtworkMetadata::fixTitleSpelling(const QString &word, const QString &replacement) {
        if (!getIsInitializedFlag()) {
            LOG_WARNING << "#" << m_ID << "attempt to fix title for not initialized artwork";
            return false;
        }

        bool result = m_MetadataModel.fixTitleSpelling(word, replacement);
        if (result) {
            markModified();
            justChanged();
        }

        return result;
    }

    std::vector<Common::KeywordItem> ArtworkMetadata::retrieveMisspelledKeywords() {
        return m_MetadataModel.retrieveMisspelledKeywords();
    }

    QStringList ArtworkMetadata::retrieveMisspelledTitleWords() {
        return m_MetadataModel.retrieveMisspelledTitleWords();
    }

    QStringList ArtworkMetadata::retrieveMisspelledDescriptionWords() {
        return m_MetadataModel.retrieveMisspelledDescriptionWords();
    }

    bool ArtworkMetadata::processFailedKeywordReplacements(const std::vector<std::shared_ptr<SpellCheck::KeywordSpellSuggestions> > &candidatesForRemoval) {
        bool result = m_MetadataModel.processFailedKeywordReplacements(candidatesForRemoval);
        if (result) {
            markModified();
        }

        return result;
    }

    void ArtworkMetadata::afterReplaceCallback() {
        m_MetadataModel.afterReplaceCallback();
    }

    Common::BasicKeywordsModel *ArtworkMetadata::getBasicKeywordsModel() {
        return &m_MetadataModel;
    }

    void ArtworkMetadata::markModified() {
        if (!getIsModifiedFlag()) {
            setIsModifiedFlag(true);
            emit modifiedChanged(true);
        }
    }

    bool ArtworkMetadata::expandPreset(size_t keywordIndex, const QStringList &presetList)  {
        bool result = m_MetadataModel.expandPreset(keywordIndex, presetList);
        if (result) {
            markModified();
            justChanged();
        }

        return result;
    }

    bool ArtworkMetadata::appendPreset(const QStringList &presetList) {
        bool result = m_MetadataModel.appendPreset(presetList);
        if (result) {
            markModified();
            justChanged();
        }

        return result;
    }

    bool ArtworkMetadata::hasKeywords(const QStringList &keywordsList) {
        bool result = m_MetadataModel.hasKeywords(keywordsList);
        return result;
    }

    void ArtworkMetadata::deepDisconnect() {
        LOG_DEBUG << "#" << m_ID;
        m_MetadataModel.disconnect();
        this->disconnect();
    }

    void ArtworkMetadata::clearSpellingInfo() {
        LOG_INTEGR_TESTS_OR_DEBUG << "#";
        m_SpellCheckInfo.clear();
    }

    void ArtworkMetadata::resetSpellingInfo() {
        LOG_DEBUG << "#" << m_ID;
        m_MetadataModel.resetSpellCheckResults();
        m_SpellCheckInfo.clear();
        m_MetadataModel.notifySpellCheckResults(Common::SpellCheckFlags::All);
    }

    void ArtworkMetadata::resetDuplicatesInfo() {
        LOG_DEBUG << "#" << m_ID;
        m_MetadataModel.resetDuplicatesInfo();
        m_SpellCheckInfo.clearDuplicates();
        m_MetadataModel.notifySpellCheckResults(Common::SpellCheckFlags::All);
    }

    void ArtworkMetadata::doOnTimer() {
        emit backupRequired();
        emit editingPaused();
    }
}

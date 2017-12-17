/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2017 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "artworkproxymodel.h"
#include <QImageReader>
#include <QSyntaxHighlighter>
#include "../Commands/commandmanager.h"
#include "../Warnings/warningsservice.h"
#include "imageartwork.h"
#include "../Models/artitemsmodel.h"
#include "../Models/videoartwork.h"
#include "../Helpers/filehelpers.h"
#include "../QMLExtensions/videocachingservice.h"
#include "../QMLExtensions/artworksupdatehub.h"

namespace Models {
    ArtworkProxyModel::ArtworkProxyModel(QObject *parent) :
        QObject(parent),
        ArtworkProxyBase(),
        m_ArtworkMetadata(nullptr)
    {
    }

    ArtworkProxyModel::~ArtworkProxyModel() {
        disconnectCurrentArtwork();
        releaseCurrentArtwork();
    }

    bool ArtworkProxyModel::getIsVideo() const {
        Models::VideoArtwork *videoArtwork = dynamic_cast<Models::VideoArtwork*>(m_ArtworkMetadata);
        bool isVideo = videoArtwork != nullptr;
        return isVideo;
    }

    void ArtworkProxyModel::setDescription(const QString &description)  {
        Q_ASSERT(getIsValid());
        LOG_INFO << description;

        if (doSetDescription(description)) {
            signalDescriptionChanged();

            if (m_ArtworkMetadata->isInitialized()) {
                doJustEdited();
            }
        }
    }

    void ArtworkProxyModel::setTitle(const QString &title) {
        Q_ASSERT(getIsValid());
        LOG_INFO << title;

        if (doSetTitle(title)) {
            signalTitleChanged();

            if (m_ArtworkMetadata->isInitialized()) {
                doJustEdited();
            }
        }
    }

    void ArtworkProxyModel::setKeywords(const QStringList &keywords) {
        Q_ASSERT(getIsValid());
        ArtworkProxyBase::setKeywords(keywords);

        if (m_ArtworkMetadata->isInitialized()) {
            doJustEdited();
        }
    }

    void ArtworkProxyModel::userDictUpdateHandler(const QStringList &keywords, bool overwritten) {
        LOG_DEBUG << "#";
        if (m_ArtworkMetadata != NULL) {
            doHandleUserDictChanged(keywords, overwritten);
        } else {
            LOG_DEBUG << "Nothing to update";
        }
    }

    void ArtworkProxyModel::userDictClearedHandler() {
        LOG_DEBUG << "#";
        if (m_ArtworkMetadata != NULL) {
            doHandleUserDictCleared();
        } else {
            LOG_DEBUG << "Nothing to update";
        }
    }

    void ArtworkProxyModel::afterSpellingErrorsFixedHandler() {
        LOG_DEBUG << "#";
#ifdef QT_DEBUG
        auto *basicModel = qobject_cast<Common::BasicMetadataModel*>(sender());
        Q_ASSERT(basicModel == getBasicMetadataModel());
#endif
        // if squeezing took place after replace
        signalKeywordsCountChanged();
    }

    void ArtworkProxyModel::onDescriptionSpellingChanged() {
        LOG_DEBUG << "#";
#ifdef QT_DEBUG
        auto *basicModel = qobject_cast<Common::BasicMetadataModel*>(sender());
        Q_ASSERT(basicModel == getBasicMetadataModel());
#endif

        emit descriptionChanged();
    }

    void ArtworkProxyModel::onTitleSpellingChanged() {
        LOG_DEBUG << "#";
#ifdef QT_DEBUG
        auto *basicModel = qobject_cast<Common::BasicMetadataModel*>(sender());
        Q_ASSERT(basicModel == getBasicMetadataModel());
#endif

        emit titleChanged();
    }

    void ArtworkProxyModel::itemUnavailableHandler(size_t index) {
        LOG_DEBUG << "#";

        if (m_ArtworkMetadata != nullptr) {
            if (index == m_ArtworkMetadata->getLastKnownIndex()) {
                LOG_INFO << "Item is not available anymore" << index;
                emit itemBecomeUnavailable();
            }
        }
    }

    void ArtworkProxyModel::editKeyword(int index, const QString &replacement) {
        doEditKeyword(index, replacement);
    }

    void ArtworkProxyModel::removeKeywordAt(int keywordIndex) {
        QString keyword;
        doRemoveKeywordAt(keywordIndex, keyword);
    }

    void ArtworkProxyModel::removeLastKeyword() {
        QString keyword;
        doRemoveLastKeyword(keyword);
    }

    bool ArtworkProxyModel::appendKeyword(const QString &keyword) {
        return doAppendKeyword(keyword);
    }

    void ArtworkProxyModel::pasteKeywords(const QStringList &keywords) {
        doAppendKeywords(keywords);
    }

    void ArtworkProxyModel::clearKeywords() {
        doClearKeywords();
    }

    QString ArtworkProxyModel::getKeywordsString() {
        return doGetKeywordsString();
    }

    void ArtworkProxyModel::suggestCorrections() {
        doSuggestCorrections();
    }

    void ArtworkProxyModel::setupDuplicatesModel() {
       doSetupDuplicatesModel();
    }

    void ArtworkProxyModel::initDescriptionHighlighting(QQuickTextDocument *document) {
        auto *highlighter = doCreateDescriptionHighligher(document);

        QObject::connect(this, &ArtworkProxyModel::descriptionSpellingChanged,
                         highlighter, &QSyntaxHighlighter::rehighlight);

        auto *basicModel = getBasicMetadataModel();
        basicModel->notifyDescriptionSpellingChanged();
    }

    void ArtworkProxyModel::initTitleHighlighting(QQuickTextDocument *document) {
        auto *highlighter = doCreateTitleHighlighter(document);

        QObject::connect(this, &ArtworkProxyModel::titleSpellingChanged,
                         highlighter, &QSyntaxHighlighter::rehighlight);

        auto *basicModel = getBasicMetadataModel();
        basicModel->notifyTitleSpellingChanged();
    }

    void ArtworkProxyModel::plainTextEdit(const QString &rawKeywords, bool spaceIsSeparator) {
        doPlainTextEdit(rawKeywords, spaceIsSeparator);
    }

    bool ArtworkProxyModel::hasTitleWordSpellError(const QString &word) {
        return getHasTitleWordSpellError(word);
    }

    bool ArtworkProxyModel::hasDescriptionWordSpellError(const QString &word) {
        return getHasDescriptionWordSpellError(word);
    }

    void ArtworkProxyModel::setSourceArtwork(QObject *artworkMetadata) {
        ArtworkMetadata *artwork = qobject_cast<ArtworkMetadata*>(artworkMetadata);
        Q_ASSERT(artwork != nullptr);
        LOG_INFO << artwork->getLastKnownIndex();

#ifdef QT_DEBUG
        auto *itemsModel = m_CommandManager->getArtItemsModel();
        Q_ASSERT(itemsModel->getArtwork(artwork->getLastKnownIndex()) == artwork);
#endif

        disconnectCurrentArtwork();
        updateCurrentArtwork();
        releaseCurrentArtwork();

        artwork->acquire();
        artwork->setIsLockedForEditing(true);
        m_ArtworkMetadata = artwork;

        connectArtworkSignals(artwork);
        updateModelProperties();
    }

    void ArtworkProxyModel::resetModel() {
        LOG_DEBUG << "#";
        disconnectCurrentArtwork();
        updateCurrentArtwork();
        releaseCurrentArtwork();
    }

    QSize ArtworkProxyModel::retrieveImageSize() const {
        Q_ASSERT(getIsValid());
        ImageArtwork *image = dynamic_cast<ImageArtwork *>(m_ArtworkMetadata);

        if (image == NULL) {
            return QSize();
        }

        QSize size;

        if (image->isInitialized()) {
            size = image->getImageSize();
        } else {
            QImageReader reader(image->getFilepath());
            size = reader.size();
            // metadata->setSize(size);
        }

        return size;
    }

    QString ArtworkProxyModel::retrieveFileSize() const {
        Q_ASSERT(getIsValid());
        qint64 size = 0;

        if (m_ArtworkMetadata->isInitialized()) {
            size = m_ArtworkMetadata->getFileSize();
        } else {
            QFileInfo fi(m_ArtworkMetadata->getFilepath());
            size = fi.size(); // in bytes
        }

        return Helpers::describeFileSize(size);
    }

    QString ArtworkProxyModel::getDateTaken() const {
        Q_ASSERT(getIsValid());
        ImageArtwork *image = dynamic_cast<ImageArtwork *>(m_ArtworkMetadata);
        if (image != NULL) {
            return image->getDateTaken();
        } else {
            return QLatin1String("");
        }
    }

    QString ArtworkProxyModel::getAttachedVectorPath() const {
        Q_ASSERT(getIsValid());
        ImageArtwork *image = dynamic_cast<ImageArtwork *>(m_ArtworkMetadata);
        if (image != NULL) {
            return image->getAttachedVectorPath();
        } else {
            return QLatin1String("");
        }
    }

    void ArtworkProxyModel::expandPreset(int keywordIndex, unsigned int presetID) {
        doExpandPreset(keywordIndex, (KeywordsPresets::ID_t)presetID);
    }

    void ArtworkProxyModel::expandLastKeywordAsPreset() {
        doExpandLastKeywordAsPreset();
    }

    void ArtworkProxyModel::addPreset(unsigned int presetID) {
        doAddPreset((KeywordsPresets::ID_t)presetID);
    }

    void ArtworkProxyModel::initSuggestion() {
        doInitSuggestion();
    }

    void ArtworkProxyModel::registerAsCurrentItem() {
        LOG_DEBUG << "#";
        doRegisterAsCurrentItem();
    }

    void ArtworkProxyModel::copyToQuickBuffer() {
        LOG_DEBUG << "#";
        doCopyToQuickBuffer();
    }

    void ArtworkProxyModel::generateCompletions(const QString &prefix) {
        LOG_DEBUG << prefix;
        doGenerateCompletions(prefix);
    }

    bool ArtworkProxyModel::acceptCompletionAsPreset(int completionID) {
        LOG_DEBUG << completionID;
        return doAcceptCompletionAsPreset(completionID);
    }

    Common::ID_t ArtworkProxyModel::getSpecialItemID() {
        Common::ID_t result = 0;

        if (m_ArtworkMetadata != nullptr) {
            result = m_ArtworkMetadata->getItemID();
        } else {
            result = SPECIAL_ID_ARTWORK_PROXY_MODEL;
        }

        return result;
    }

    void ArtworkProxyModel::connectArtworkSignals(ArtworkMetadata *artwork) {
        auto *basicModel = artwork->getBasicModel();

        QObject::connect(basicModel, &Common::BasicMetadataModel::descriptionSpellingChanged,
                         this, &ArtworkProxyModel::onDescriptionSpellingChanged);
        QObject::connect(basicModel, &Common::BasicMetadataModel::titleSpellingChanged,
                         this, &ArtworkProxyModel::onTitleSpellingChanged);

        QObject::connect(basicModel, &Common::BasicMetadataModel::completionsAvailable,
                         this, &ArtworkProxyModel::completionsAvailable);

        QObject::connect(basicModel, &Common::BasicMetadataModel::afterSpellingErrorsFixed,
                         this, &ArtworkProxyModel::afterSpellingErrorsFixedHandler);

        QObject::connect(basicModel, &Common::BasicMetadataModel::descriptionSpellingChanged,
                         this, &ArtworkProxyModel::descriptionSpellingChanged);
        QObject::connect(basicModel, &Common::BasicMetadataModel::titleSpellingChanged,
                         this, &ArtworkProxyModel::titleSpellingChanged);
        QObject::connect(basicModel, &Common::BasicMetadataModel::keywordsSpellingChanged,
                         this, &ArtworkProxyModel::keywordsSpellingChanged);

        QObject::connect(artwork, SIGNAL(thumbnailUpdated()),
                         this, SIGNAL(thumbnailChanged()));
    }

    void ArtworkProxyModel::updateModelProperties() {
        emit descriptionChanged();
        emit titleChanged();
        emit keywordsCountChanged();
        emit thumbnailChanged();
        emit imagePathChanged();
        emit titleSpellingChanged();
        emit descriptionSpellingChanged();
        emit keywordsSpellingChanged();

        Q_ASSERT(m_ArtworkMetadata != nullptr);
        m_PropertiesMap.updateProperties(m_ArtworkMetadata);

        emit isValidChanged();
    }

    void ArtworkProxyModel::updateCurrentArtwork() {
        LOG_DEBUG << "#";
        if (m_ArtworkMetadata == nullptr) { return; }

        size_t lastKnownIndex = m_ArtworkMetadata->getLastKnownIndex();
        LOG_DEBUG << "index:" << lastKnownIndex;

#ifndef CORE_TESTS
        Models::VideoArtwork *videoArtwork = dynamic_cast<Models::VideoArtwork*>(m_ArtworkMetadata);
        if (videoArtwork != nullptr) {
            if (!videoArtwork->isThumbnailGenerated()) {
                auto *videoCachingService = m_CommandManager->getVideoCachingService();
                videoCachingService->generateThumbnail(videoArtwork);
            } else {
                auto *updateHub = m_CommandManager->getArtworksUpdateHub();
                updateHub->updateArtwork(videoArtwork->getItemID(),
                                         videoArtwork->getLastKnownIndex(),
                                         QSet<int>() << ArtItemsModel::ArtworkThumbnailRole);
            }
        }
#endif

        xpiks()->updateArtworksAtIndices(QVector<int>() << (int)lastKnownIndex);

        xpiks()->submitForWarningsCheck(m_ArtworkMetadata);
        xpiks()->checkSemanticDuplicates(m_ArtworkMetadata->getBasicModel());

        emit warningsCouldHaveChanged(lastKnownIndex);
        emit duplicatesCouldHaveChanged(lastKnownIndex);
    }

    void ArtworkProxyModel::disconnectCurrentArtwork() {
        LOG_DEBUG << "#";
        if (m_ArtworkMetadata != nullptr) {
            auto *basicModel = m_ArtworkMetadata->getBasicModel();
            basicModel->disconnect(this);
            this->disconnect(basicModel);
        }
    }

    void ArtworkProxyModel::releaseCurrentArtwork() {
        LOG_DEBUG << "#";
        if (m_ArtworkMetadata != nullptr) {
            m_ArtworkMetadata->setIsLockedForEditing(false);
            m_ArtworkMetadata->release();
            LOG_DEBUG << "Metadata released";
        }

        m_ArtworkMetadata = nullptr;
        emit isValidChanged();
    }
}

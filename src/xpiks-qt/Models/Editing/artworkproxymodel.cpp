/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "artworkproxymodel.h"

#include <QFileInfo>
#include <QImageReader>
#include <QLatin1String>
#include <QQmlEngine>
#include <QVector>
#include <QtDebug>
#include <QtGlobal>

#include "Artworks/artworkmetadata.h"
#include "Artworks/basickeywordsmodel.h"
#include "Artworks/imageartwork.h"
#include "Artworks/videoartwork.h"
#include "Common/defines.h"
#include "Common/logging.h"
#include "Common/messages.h"
#include "Common/types.h"
#include "Helpers/filehelpers.h"
#include "KeywordsPresets/presetmodel.h"
#include "Models/Artworks/artworkslistmodel.h"
#include "Models/Editing/artworkproxybase.h"
#include "Models/Editing/currenteditableproxyartwork.h"
#include "Models/keyvaluelist.h"
#include "Services/iartworksupdater.h"

namespace Models {
    ArtworkProxyModel::ArtworkProxyModel(Commands::ICommandManager &commandManager,
                                         KeywordsPresets::IPresetsManager &presetsManager,
                                         Services::IArtworksUpdater &artworksUpdater,
                                         QObject *parent) :
        QObject(parent),
        ArtworkProxyBase(),
        m_ArtworkMetadata(nullptr),
        m_ProxyIndex(-1),
        m_CommandManager(commandManager),
        m_PresetsManager(presetsManager),
        m_ArtworksUpdater(artworksUpdater)
    {
    }

    ArtworkProxyModel::~ArtworkProxyModel() {
        LOG_DEBUG << "#";
        disconnectCurrentArtwork();
        releaseCurrentArtwork();
    }

    bool ArtworkProxyModel::getIsVideo() const {
        auto videoArtwork = std::dynamic_pointer_cast<Artworks::VideoArtwork>(m_ArtworkMetadata);
        bool isVideo = videoArtwork != nullptr;
        return isVideo;
    }

    QString ArtworkProxyModel::getThumbPath() const { return m_ArtworkMetadata->getThumbnailPath(); }
    const QString &ArtworkProxyModel::getFilePath() const { return m_ArtworkMetadata->getFilepath(); }
    QString ArtworkProxyModel::getBasename() const { return m_ArtworkMetadata->getBaseFilename(); }

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
        auto *basicModel = qobject_cast<Artworks::BasicMetadataModel*>(sender());
        Q_ASSERT(basicModel == getBasicMetadataModel());
#endif
        // if squeezing took place after replace
        signalKeywordsCountChanged();
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

    bool ArtworkProxyModel::moveKeyword(int from, int to) {
        return doMoveKeyword(from, to);
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

    bool ArtworkProxyModel::hasTitleWordSpellError(const QString &word) {
        return getHasTitleWordSpellError(word);
    }

    bool ArtworkProxyModel::hasDescriptionWordSpellError(const QString &word) {
        return getHasDescriptionWordSpellError(word);
    }

    void ArtworkProxyModel::setSourceArtwork(const std::shared_ptr<Artworks::ArtworkMetadata> &artwork, int proxyIndex) {
        Q_ASSERT(artwork != nullptr);
        LOG_INFO << artwork->getLastKnownIndex();

        disconnectCurrentArtwork();
        updateCurrentArtwork();
        releaseCurrentArtwork();

        artwork->setIsLockedForEditing(true);
        m_ArtworkMetadata = artwork;
        m_ProxyIndex = proxyIndex;

        connectArtworkSignals(artwork.get());
        updateModelProperties();

        sendMessage(std::make_shared<CurrentEditableProxyArtwork>(*this));
    }

    void ArtworkProxyModel::resetModel() {
        LOG_DEBUG << "#";
        disconnectCurrentArtwork();
        updateCurrentArtwork();
        releaseCurrentArtwork();
    }

    QObject *ArtworkProxyModel::getBasicModelObject() {
        QObject *item = getBasicMetadataModel();
        QQmlEngine::setObjectOwnership(item, QQmlEngine::CppOwnership);
        return item;
    }

    QObject *ArtworkProxyModel::getPropertiesMap() {
        QObject *item = &m_PropertiesMap;
        QQmlEngine::setObjectOwnership(item, QQmlEngine::CppOwnership);
        return item;
    }

    QSize ArtworkProxyModel::retrieveImageSize() const {
        Q_ASSERT(getIsValid());
        auto image = std::dynamic_pointer_cast<Artworks::ImageArtwork>(m_ArtworkMetadata);

        if (image == nullptr) {
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
        auto image = std::dynamic_pointer_cast<Artworks::ImageArtwork>(m_ArtworkMetadata);
        if (image != nullptr) {
            return image->getDateTaken();
        } else {
            return QLatin1String("");
        }
    }

    QString ArtworkProxyModel::getAttachedVectorPath() const {
        Q_ASSERT(getIsValid());
        auto image = std::dynamic_pointer_cast<Artworks::ImageArtwork>(m_ArtworkMetadata);
        if (image != nullptr) {
            return image->getAttachedVectorPath();
        } else {
            return QLatin1String("");
        }
    }

    void ArtworkProxyModel::expandPreset(int keywordIndex, unsigned int presetID) {
        doExpandPreset(keywordIndex, (KeywordsPresets::ID_t)presetID, m_PresetsManager);
    }

    void ArtworkProxyModel::expandLastKeywordAsPreset() {
        doExpandLastKeywordAsPreset(m_PresetsManager);
    }

    void ArtworkProxyModel::addPreset(unsigned int presetID) {
        doAppendPreset((KeywordsPresets::ID_t)presetID, m_PresetsManager);
    }

#ifdef UI_TESTS
    void ArtworkProxyModel::clearModel() {
        LOG_DEBUG << "#";
        getBasicMetadataModel()->clearModel();
        updateModelProperties();
    }

    void ArtworkProxyModel::setupModel() {
        sendMessage(std::make_shared<CurrentEditableProxyArtwork>(*this));
    }
#endif

    bool ArtworkProxyModel::acceptCompletionAsPreset(AutoComplete::ICompletionSource &completionSource, int completionID) {
        LOG_DEBUG << completionID;
        const bool accepted = doAcceptCompletionAsPreset(completionID, completionSource, m_PresetsManager);
        return accepted;
    }

    void ArtworkProxyModel::handleMessage(const UnavailableFilesMessage &) {
        LOG_DEBUG << "#";

        if (m_ArtworkMetadata != nullptr) {
            if (m_ArtworkMetadata->isUnavailable()) {
                LOG_INFO << "Item is not available anymore";
                emit itemBecomeUnavailable();
            }
        }
    }

    Artworks::BasicMetadataModel *ArtworkProxyModel::getBasicMetadataModel() {
        Q_ASSERT(m_ArtworkMetadata != nullptr);
        return &m_ArtworkMetadata->getBasicMetadataModel();
    }

    Artworks::IArtworkMetadata *ArtworkProxyModel::getArtworkMetadata() {
        Q_ASSERT(m_ArtworkMetadata != nullptr);
        return m_ArtworkMetadata.get();
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

    void ArtworkProxyModel::submitForInspection() {
        Q_ASSERT(m_ArtworkMetadata != nullptr);
        if (m_ArtworkMetadata == nullptr) { return; }
        sendMessage(Common::NamedType<std::shared_ptr<Artworks::IBasicModelSource>, Common::MessageType::SpellCheck>(m_ArtworkMetadata));
    }

    void ArtworkProxyModel::connectArtworkSignals(Artworks::ArtworkMetadata *artwork) {
        auto &basicModel = artwork->getBasicMetadataModel();

        QObject::connect(&basicModel, &Artworks::BasicMetadataModel::afterSpellingErrorsFixed,
                         this, &ArtworkProxyModel::afterSpellingErrorsFixedHandler);

        QObject::connect(&basicModel, &Artworks::BasicMetadataModel::descriptionSpellingChanged,
                         this, &ArtworkProxyModel::descriptionSpellingChanged);
        QObject::connect(&basicModel, &Artworks::BasicMetadataModel::titleSpellingChanged,
                         this, &ArtworkProxyModel::titleSpellingChanged);
        QObject::connect(&basicModel, &Artworks::BasicMetadataModel::keywordsSpellingChanged,
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
        emit artworkChanged();

        Q_ASSERT(m_ArtworkMetadata != nullptr);
        m_PropertiesMap.updateProperties(m_ArtworkMetadata);

        emit isValidChanged();
    }

    void ArtworkProxyModel::updateCurrentArtwork() {
        LOG_DEBUG << "#";
        if (m_ArtworkMetadata == nullptr) { return; }

        // TODO: consider refactoring this to a command template??
        m_ArtworksUpdater.updateArtwork(m_ArtworkMetadata);
        //submitForInspection();

        auto videoArtwork = std::dynamic_pointer_cast<Artworks::VideoArtwork>(m_ArtworkMetadata);
        if (videoArtwork != nullptr) {
            if (!videoArtwork->isThumbnailGenerated()) {
                sendMessage(videoArtwork);
            } else {
                m_ArtworksUpdater.updateArtworkByID(videoArtwork->getItemID(),
                                                    videoArtwork->getLastKnownIndex(),
                                                    QVector<int>() << Models::ArtworksListModel::ArtworkThumbnailRole);
            }
        }

        emit warningsCouldHaveChanged(m_ArtworkMetadata->getLastKnownIndex());
        emit duplicatesCouldHaveChanged(m_ArtworkMetadata->getLastKnownIndex());
    }

    void ArtworkProxyModel::disconnectCurrentArtwork() {
        LOG_DEBUG << "#";
        if (m_ArtworkMetadata != nullptr) {
            auto &basicModel = m_ArtworkMetadata->getBasicMetadataModel();
            m_ArtworkMetadata->disconnect(this);
            basicModel.disconnect(this);
            this->disconnect(&basicModel);
            this->disconnect(m_ArtworkMetadata.get());
        }
    }

    void ArtworkProxyModel::releaseCurrentArtwork() {
        LOG_DEBUG << "#";
        if (m_ArtworkMetadata != nullptr) {
            m_ArtworkMetadata->setIsLockedForEditing(false);
            LOG_DEBUG << "Metadata released";
        }

        m_ArtworkMetadata = nullptr;
        m_ProxyIndex = -1;
        sendMessage(std::shared_ptr<ICurrentEditable>());
        emit isValidChanged();
    }
}

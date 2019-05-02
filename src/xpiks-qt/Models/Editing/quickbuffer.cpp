/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "quickbuffer.h"

#include <QQmlEngine>
#include <QtDebug>

#include "Artworks/basickeywordsmodel.h"
#include "Artworks/basicmetadatamodel.h"
#include "Artworks/basicmodelsource.h"
#include "Commands/Base/icommandmanager.h"
#include "Common/delayedactionentity.h"
#include "Common/logging.h"
#include "Models/Editing/artworkproxybase.h"
#include "Models/Editing/currenteditablemodel.h"
#include "Models/Editing/icurrenteditable.h"
#include "Models/Editing/quickbuffermessage.h"

#define MAX_EDITING_PAUSE_RESTARTS 10
#define QUICKBUFFER_EDITING_PAUSE 1000

namespace Models {
    QuickBuffer::QuickBuffer(CurrentEditableModel &currentEditableModel,
                             Commands::ICommandManager &commandManager,
                             QObject *parent) :
        QObject(parent),
        ArtworkProxyBase(),
        Common::DelayedActionEntity(QUICKBUFFER_EDITING_PAUSE, MAX_EDITING_PAUSE_RESTARTS),
        m_BasicModel(m_SpellCheckInfo, this),
        m_CurrentEditableModel(currentEditableModel),
        m_CommandManager(commandManager)
    {
        QObject::connect(&m_BasicModel, &Artworks::BasicMetadataModel::titleSpellingChanged,
                         this, &QuickBuffer::titleSpellingChanged);
        QObject::connect(&m_BasicModel, &Artworks::BasicMetadataModel::descriptionSpellingChanged,
                         this, &QuickBuffer::descriptionSpellingChanged);
        QObject::connect(&m_BasicModel, &Artworks::BasicMetadataModel::keywordsSpellingChanged,
                         this, &QuickBuffer::keywordsSpellingChanged);

//        QObject::connect(&m_BasicModel, SIGNAL(completionsAvailable()),
//                         this, SIGNAL(completionsAvailable()));

        QObject::connect(&m_BasicModel, &Artworks::BasicMetadataModel::afterSpellingErrorsFixed,
                         this, &QuickBuffer::afterSpellingErrorsFixedHandler);
    }

    void QuickBuffer::afterSpellingErrorsFixedHandler() {
        // if squeezing took place after replace
        signalKeywordsCountChanged();
        emit descriptionSpellingChanged();
        emit titleSpellingChanged();
        emit keywordsSpellingChanged();
    }

    void QuickBuffer::userDictUpdateHandler(const QStringList &keywords, bool overwritten) {
        LOG_DEBUG << "#";
        doHandleUserDictChanged(keywords, overwritten);
    }

    void QuickBuffer::userDictClearedHandler() {
        LOG_DEBUG << "#";
        doHandleUserDictCleared();
    }

    void QuickBuffer::removeKeywordAt(int keywordIndex) {
        QString keyword;
        doRemoveKeywordAt(keywordIndex, keyword);
        emit isEmptyChanged();
    }

    void QuickBuffer::removeLastKeyword() {
        QString keyword;
        doRemoveLastKeyword(keyword);
        emit isEmptyChanged();
    }

    bool QuickBuffer::appendKeyword(const QString &keyword) {
        bool added = doAppendKeyword(keyword);
        emit isEmptyChanged();
        return added;
    }

    void QuickBuffer::pasteKeywords(const QStringList &keywords) {
        doAppendKeywords(keywords);
        emit isEmptyChanged();
    }

    void QuickBuffer::clearKeywords() {
        doClearKeywords();
        emit isEmptyChanged();
    }

    QString QuickBuffer::getKeywordsString() {
        return doGetKeywordsString();
    }

    bool QuickBuffer::hasTitleWordSpellError(const QString &word) {
        return getHasTitleWordSpellError(word);
    }

    bool QuickBuffer::hasDescriptionWordSpellError(const QString &word) {
        return getHasDescriptionWordSpellError(word);
    }

    void QuickBuffer::resetModel() {
        LOG_DEBUG << "#";
        m_BasicModel.clearModel();
        emit isEmptyChanged();
    }

    QObject *QuickBuffer::getBasicModelObject() {
        QObject *item = getBasicMetadataModel();
        QQmlEngine::setObjectOwnership(item, QQmlEngine::CppOwnership);

        return item;
    }

    bool QuickBuffer::copyToCurrentEditable() {
        LOG_DEBUG << "#";
        bool result = false;
        auto currentEditable = m_CurrentEditableModel.getCurrentEditable();
        if (currentEditable) {
            auto command = currentEditable->applyEdits(getTitle(), getDescription(), getKeywords());
            m_CommandManager.processCommand(command);
            result = true;
        } else {
            LOG_WARNING << "Nothing registered as current item";
            sendMessage(Connectivity::EventType::CurrentItemMiss);
        }

        return result;
    }

    bool QuickBuffer::copyFromCurrentEditable() {
        LOG_DEBUG << "#";
        bool result = false;
        auto currentEditable = m_CurrentEditableModel.getCurrentEditable();
        if (currentEditable) {
            QString title = currentEditable->getTitle();
            if (!title.isEmpty()) { this->setTitle(title); }

            QString description = currentEditable->getDescription();
            if (!description.isEmpty()) { this->setDescription(description); }

            QStringList keywords = currentEditable->getKeywords();
            if (!keywords.empty()) { this->setKeywords(keywords); }

            result = true;
        } else {
            LOG_WARNING << "Nothing registered as current item";
            sendMessage(Connectivity::EventType::CurrentItemMiss);
        }

        return result;
    }

    bool QuickBuffer::getIsEmpty() {
        auto *model = getBasicMetadataModel();
        bool result = model->isTitleEmpty() && model->isDescriptionEmpty() && model->areKeywordsEmpty();
        return result;
    }

    void QuickBuffer::handleMessage(const QuickBufferMessage &message) {
        LOG_DEBUG << "#";
        const bool overwrite = message.m_Override;

        if (!message.m_Title.isEmpty() || overwrite) { this->setTitle(message.m_Title); }
        if (!message.m_Description.isEmpty() || overwrite) { this->setDescription(message.m_Description); }
        if (!message.m_Keywords.empty() || overwrite) { this->setKeywords(message.m_Keywords); }

        emit isEmptyChanged();
    }

    void QuickBuffer::submitForInspection() {
        sendMessage(BasicSpellCheckMessageType(
                        std::make_shared<Artworks::BasicModelSource>(m_BasicModel)));
    }

    void QuickBuffer::doJustEdited() {
        ArtworkProxyBase::doJustEdited();
        justChanged();
    }

    void QuickBuffer::doOnTimer() {
        LOG_DEBUG << "#";
        submitForInspection();
    }
}

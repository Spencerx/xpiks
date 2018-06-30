/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "quickbuffer.h"
#include <Commands/Base/icommandmanager.h>
#include <Models/Editing/icurrenteditable.h>
#include <Models/Editing/currenteditablemodel.h>
#include <Commands/Editing/modifyartworkscommand.h>
#include <Commands/Base/compositecommandtemplate.h>
#include <Commands/Editing/editartworkstemplate.h>
#include <Commands/appmessages.h>

#define MAX_EDITING_PAUSE_RESTARTS 10
#define QUICKBUFFER_EDITING_PAUSE 1000

namespace Models {
    QuickBuffer::QuickBuffer(Commands::AppMessages &messages,
                             CurrentEditableModel &currentEditableModel,
                             Commands::ICommandManager &commandManager,
                             QObject *parent) :
        QObject(parent),
        ArtworkProxyBase(),
        Common::DelayedActionEntity(QUICKBUFFER_EDITING_PAUSE, MAX_EDITING_PAUSE_RESTARTS),
        m_BasicModel(m_HoldPlaceholder, this),
        m_CurrentEditableModel(currentEditableModel),
        m_Messages(messages),
        m_CommandManager(commandManager)
    {
        m_BasicModel.setSpellCheckInfo(&m_SpellCheckInfo);

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

        m_Messages
                .ofType<QString, QString, QStringList>()
                .withID(Commands::AppMessages::CopyToQuickBuffer)
                .addListener(std::bind(&QuickBuffer::setQuickBuffer, this));
    }

    QuickBuffer::~QuickBuffer() {
        // BUMP
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
        }

        return result;
    }

    bool QuickBuffer::getIsEmpty() {
        auto *model = getBasicMetadataModel();
        bool result = model->isTitleEmpty() && model->isDescriptionEmpty() && model->areKeywordsEmpty();
        return result;
    }

    void QuickBuffer::setQuickBuffer(const QString &title,
                                     const QString &description,
                                     const QStringList &keywords,
                                     bool overwrite) {
        LOG_DEBUG << "#";
        if (!title.isEmpty() || overwrite) { this->setTitle(title); }
        if (!description.isEmpty() || overwrite) { this->setDescription(description); }
        if (!keywords.empty() || overwrite) { this->setKeywords(keywords); }

        emit isEmptyChanged();
    }

    void QuickBuffer::doJustEdited() {
        ArtworkProxyBase::doJustEdited();
        justChanged();
    }

    void QuickBuffer::doOnTimer() {
        LOG_DEBUG << "#";
        xpiks()->submitItemForSpellCheck(&m_BasicModel);
    }
}

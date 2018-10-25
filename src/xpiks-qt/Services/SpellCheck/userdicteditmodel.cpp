/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "userdicteditmodel.h"

#include <QQmlEngine>
#include <QtDebug>

#include "Artworks/basicmetadatamodel.h"
#include "Common/logging.h"
#include "Models/Editing/artworkproxybase.h"
#include "Services/SpellCheck/userdictionary.h"

namespace SpellCheck {
    UserDictEditModel::UserDictEditModel(UserDictionary &userDictionary, QObject *parent):
        QObject(parent),
        ArtworkProxyBase(),
        m_BasicModel(m_SpellCheckInfo, this),
        m_UserDictionary(userDictionary)
    {
    }

    void UserDictEditModel::initializeModel() {
        LOG_DEBUG << "#";
        auto keywordsList = m_UserDictionary.getWords();
        m_BasicModel.setKeywords(keywordsList);
    }

    void UserDictEditModel::removeKeywordAt(int keywordIndex) {
        LOG_INFO << keywordIndex;
        QString keyword;
        doRemoveKeywordAt(keywordIndex, keyword);
    }

    void UserDictEditModel::removeLastKeyword() {
        LOG_DEBUG << "#";
        QString keyword;
        doRemoveLastKeyword(keyword);
    }

    void UserDictEditModel::appendKeyword(const QString &keyword) {
        LOG_INFO << keyword;
        doAppendKeyword(keyword);
    }

    void UserDictEditModel::clearKeywords() {
        LOG_DEBUG << "#";
        doClearKeywords();
    }

    void UserDictEditModel::clearModel() {
        LOG_DEBUG << "#";
        m_BasicModel.clearModel();
    }

    QObject *UserDictEditModel::getBasicModelObject() {
        QObject *item = getBasicMetadataModel();
        QQmlEngine::setObjectOwnership(item, QQmlEngine::CppOwnership);
        return item;
    }

    void UserDictEditModel::saveUserDict() {
        LOG_DEBUG << "#";

        auto keywords = m_BasicModel.getKeywords();
        m_UserDictionary.reset(keywords);
    }
}

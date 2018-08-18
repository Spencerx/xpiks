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
#include <Helpers/keywordshelpers.h>
#include "userdictionary.h"

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

    void UserDictEditModel::resetModel() {
        LOG_DEBUG << "#";
        m_BasicModel.clearModel();
        m_UserDictionary.clear();
    }

    void UserDictEditModel::saveUserDict() {
        LOG_DEBUG << "#";

        auto keywords = m_BasicModel.getKeywords();
        m_UserDictionary.reset(keywords);
    }
}

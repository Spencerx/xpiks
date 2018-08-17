/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SPELLCHECKUSERDICT_H
#define SPELLCHECKUSERDICT_H

#include <QObject>
#include <QQmlEngine>
#include <Models/Editing/artworkproxybase.h>
#include <Artworks/basicmetadatamodel.h>
#include <Services/SpellCheck/spellcheckiteminfo.h>

namespace SpellCheck {
    class UserDictionary;

    class UserDictEditModel: public QObject, public Models::ArtworkProxyBase
    {
        Q_OBJECT
        Q_PROPERTY(int keywordsCount READ getKeywordsCount NOTIFY keywordsCountChanged)

    public:
        UserDictEditModel(UserDictionary &userDictionary, QObject *parent=0);

    signals:
        void keywordsCountChanged();

    protected:
        virtual void signalDescriptionChanged() override { /* BUMP */ }
        virtual void signalTitleChanged() override { /* BUMP */ }
        virtual void signalKeywordsCountChanged() override { emit keywordsCountChanged(); }

    public:
        Q_INVOKABLE void initializeModel();
        Q_INVOKABLE void removeKeywordAt(int keywordIndex);
        Q_INVOKABLE void removeLastKeyword();
        Q_INVOKABLE void appendKeyword(const QString &keyword);
        Q_INVOKABLE void clearKeywords();
        Q_INVOKABLE void resetModel();
        Q_INVOKABLE QObject *getBasicModel() {
            QObject *item = getBasicMetadataModel();
            QQmlEngine::setObjectOwnership(item, QQmlEngine::CppOwnership);

            return item;
        }
        Q_INVOKABLE void saveUserDict();


    protected:
        virtual Artworks::BasicMetadataModel *getBasicMetadataModel() override { return &m_BasicModel; }
        virtual Artworks::IArtworkMetadata *getArtworkMetadata() override { return &m_BasicModel; }

    protected:
        virtual void submitForInspection() override { /*BUMP*/ }

    private:
        Common::Hold m_HoldPlaceholder;
        Artworks::BasicMetadataModel m_BasicModel;
        SpellCheck::SpellCheckItemInfo m_SpellCheckInfo;
        UserDictionary &m_UserDictionary;
    };
}
#endif // SPELLCHECKUSERDICT_H

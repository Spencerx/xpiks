/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef ARTWORKPROXYBASE_H
#define ARTWORKPROXYBASE_H

#include "../Artworks/basicmetadatamodel.h"
#include "../Artworks/imetadataoperator.h"
#include "../KeywordsPresets/ipresetsmanager.h"
#include "../QuickBuffer/currenteditableproxyartwork.h"
#include "../Common/types.h"

namespace AutoComplete {
    class ICompletionSource;
}

namespace Models {
    class ArtworkProxyBase
    {
    public:
        ArtworkProxyBase(AutoComplete::ICompletionSource &completionSource,
                         KeywordsPresets::IPresetsManager &presetsManager);
        virtual ~ArtworkProxyBase() {}

    public:
        QString getDescription();
        QString getTitle();
        QStringList getKeywords();
        int getKeywordsCount();
        virtual void setDescription(const QString &description);
        virtual void setTitle(const QString &title);
        virtual void setKeywords(const QStringList &keywords);

    protected:
        virtual bool doSetDescription(const QString &description);
        virtual bool doSetTitle(const QString &title);
        virtual void doSetKeywords(const QStringList &keywords);

        // TODO: virtual inheritance from qobject is not supported
    //signals:
        virtual void signalDescriptionChanged() = 0;
        virtual void signalTitleChanged() = 0;
        virtual void signalKeywordsCountChanged() = 0;

    protected:
        virtual Artworks::BasicMetadataModel *getBasicMetadataModel() = 0;
        virtual Common::IMetadataOperator *getMetadataOperator() = 0;

    protected:
        virtual void submitForInspection() = 0;

    protected:
        virtual Common::ID_t getSpecialItemID();

    protected:
        bool doEditKeyword(int index, const QString &replacement);
        bool doRemoveKeywordAt(int keywordIndex, QString &keyword);
        bool doRemoveLastKeyword(QString &keyword);
        bool doAppendKeyword(const QString &keyword);
        size_t doAppendKeywords(const QStringList &keywords);
        bool doRemoveKeywords(const QSet<QString> &keywords, bool caseSensitive);
        bool doMoveKeyword(int from, int to);
        bool doClearKeywords();
        QString doGetKeywordsString();

    protected:
        void doPlainTextEdit(const QString &rawKeywords, bool spaceIsSeparator);
        bool getHasTitleWordSpellError(const QString &word);
        bool getHasDescriptionWordSpellError(const QString &word);
        bool getHasTitleSpellErrors();
        bool getHasDescriptionSpellError();
        bool getHasKeywordsSpellError();

    protected:
        bool doExpandPreset(int keywordIndex, KeywordsPresets::ID_t presetID);
        bool doAppendPreset(KeywordsPresets::ID_t presetID);
        bool doExpandLastKeywordAsPreset();
        bool doAddPreset(KeywordsPresets::ID_t presetID);
        bool doRemovePreset(KeywordsPresets::ID_t presetID);

    protected:
        void doHandleUserDictChanged(const QStringList &keywords, bool overwritten);
        void doHandleUserDictCleared();
        void doCopyToQuickBuffer();
        bool hasKeywords(const QStringList &keywordsList);
        virtual void doJustEdited();
        bool doAcceptCompletionAsPreset(int completionID);

    private:
        friend class QuickBuffer::CurrentEditableProxyArtwork;

    protected:
        AutoComplete::ICompletionSource &m_CompletionSource;
        KeywordsPresets::IPresetsManager &m_PresetsManager;
    };
}

#endif // ARTWORKPROXYBASE_H

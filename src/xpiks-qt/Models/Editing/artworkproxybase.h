/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef ARTWORKPROXYBASE_H
#define ARTWORKPROXYBASE_H

#include <cstddef>

#include <QSet>
#include <QString>
#include <QStringList>

#include "Common/types.h"
#include "KeywordsPresets/presetmodel.h"

namespace Artworks {
    class BasicMetadataModel;
    class IArtworkMetadata;
}

namespace KeywordsPresets {
    class IPresetsManager;
}

namespace AutoComplete {
    class ICompletionSource;
}

namespace Models {
    class ArtworkProxyBase
    {
    public:
        ArtworkProxyBase();
        virtual ~ArtworkProxyBase() { }

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
        virtual Artworks::IArtworkMetadata *getArtworkMetadata() = 0;

    protected:
        virtual void submitForInspection() = 0;
        virtual void doJustEdited() { }

    protected:
        virtual Common::ID_t getSpecialItemID();

    protected:
        bool doEditKeyword(int index, const QString &replacement);
        bool doRemoveKeywordAt(int index, QString &keyword);
        bool doRemoveLastKeyword(QString &keyword);
        bool doAppendKeyword(const QString &keyword);
        size_t doAppendKeywords(const QStringList &keywords);
        bool doRemoveKeywords(const QSet<QString> &keywords, bool caseSensitive);
        bool doMoveKeyword(int from, int to);
        bool doClearKeywords();
        QString doGetKeywordsString();

    protected:
        bool getHasTitleWordSpellError(const QString &word);
        bool getHasDescriptionWordSpellError(const QString &word);
        bool getHasTitleSpellErrors();
        bool getHasDescriptionSpellError();
        bool getHasKeywordsSpellError();

    protected:
        bool doExpandPreset(int keywordIndex,
                            KeywordsPresets::ID_t presetID,
                            KeywordsPresets::IPresetsManager &presetsManager);
        bool doAppendPreset(KeywordsPresets::ID_t presetID,
                            KeywordsPresets::IPresetsManager &presetsManager);
        bool doExpandLastKeywordAsPreset(KeywordsPresets::IPresetsManager &presetsManager);
        bool doRemovePreset(KeywordsPresets::ID_t presetID, KeywordsPresets::IPresetsManager &presetsManager);

    protected:
        void doHandleUserDictChanged(const QStringList &keywords, bool overwritten);
        void doHandleUserDictCleared();
        bool hasKeywords(const QStringList &keywordsList);
        bool doAcceptCompletionAsPreset(int completionID,
                                        AutoComplete::ICompletionSource &completionSource,
                                        KeywordsPresets::IPresetsManager &presetsManager);

    private:
        friend class CurrentEditableProxyArtwork;
    };
}

#endif // ARTWORKPROXYBASE_H

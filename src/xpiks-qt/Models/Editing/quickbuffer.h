/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef QUICKBUFFER_H
#define QUICKBUFFER_H

#include <QObject>
#include <QQmlEngine>
#include <Models/Editing/artworkproxybase.h>
#include <Artworks/basicmetadatamodel.h>
#include <Services/SpellCheck/spellcheckiteminfo.h>
#include <Suggestion/suggestionartwork.h>
#include <Common/hold.h>
#include <Common/delayedactionentity.h>
#include <Common/events.h>

namespace Commands {
    class ICommandManager;
}

namespace Models {
    class CurrentEditableModel;

    class QuickBuffer:
            public QObject,
            public Models::ArtworkProxyBase,
            public Common::DelayedActionEntity,
            public Common::EventsSource<Artworks::BasicKeywordsModel*>
    {
        Q_OBJECT
        Q_PROPERTY(QString description READ getDescription WRITE setDescription NOTIFY descriptionChanged)
        Q_PROPERTY(QString title READ getTitle WRITE setTitle NOTIFY titleChanged)
        Q_PROPERTY(int keywordsCount READ getKeywordsCount NOTIFY keywordsCountChanged)
        Q_PROPERTY(bool isEmpty READ getIsEmpty NOTIFY isEmptyChanged)
        Q_PROPERTY(bool hasTitleSpellErrors READ getHasTitleSpellErrors NOTIFY titleSpellingChanged)
        Q_PROPERTY(bool hasDescriptionSpellErrors READ getHasDescriptionSpellError NOTIFY descriptionSpellingChanged)
        Q_PROPERTY(bool hasKeywordsSpellErrors READ getHasKeywordsSpellError NOTIFY keywordsSpellingChanged)

    public:
        explicit QuickBuffer(CurrentEditableModel &currentEditableModel,
                             Commands::ICommandManager &commandManager,
                             QObject *parent = 0);

    signals:
        void descriptionChanged();
        void titleChanged();
        void keywordsCountChanged();
        void isEmptyChanged();
        void titleSpellingChanged();
        void descriptionSpellingChanged();
        void keywordsSpellingChanged();

    protected:
        virtual void signalDescriptionChanged() override { emit descriptionChanged(); emit isEmptyChanged(); }
        virtual void signalTitleChanged() override { emit titleChanged(); emit isEmptyChanged(); }
        virtual void signalKeywordsCountChanged() override { emit keywordsCountChanged(); emit isEmptyChanged(); }

    public slots:
        void afterSpellingErrorsFixedHandler();
        void userDictUpdateHandler(const QStringList &keywords, bool overwritten);
        void userDictClearedHandler();

    public:
        Q_INVOKABLE void removeKeywordAt(int keywordIndex);
        Q_INVOKABLE void removeLastKeyword();
        Q_INVOKABLE bool appendKeyword(const QString &keyword);
        Q_INVOKABLE void pasteKeywords(const QStringList &keywords);
        Q_INVOKABLE void clearKeywords();
        Q_INVOKABLE QString getKeywordsString();
        Q_INVOKABLE bool hasTitleWordSpellError(const QString &word);
        Q_INVOKABLE bool hasDescriptionWordSpellError(const QString &word);
        Q_INVOKABLE void resetModel();
        Q_INVOKABLE QObject *getBasicModel() {
            QObject *item = getBasicMetadataModel();
            QQmlEngine::setObjectOwnership(item, QQmlEngine::CppOwnership);

            return item;
        }
        Q_INVOKABLE bool copyToCurrentEditable();

#ifdef INTEGRATION_TESTS
    public:
        bool hasSpellErrors() { return m_BasicModel.hasSpellErrors(); }
#endif

    public:
        bool getIsEmpty();

    private:
        void setQuickBuffer(const QString &title,
                            const QString &description,
                            const QStringList &keywords,
                            bool overwrite);

    protected:
        virtual Artworks::BasicMetadataModel *getBasicMetadataModel() override { return &m_BasicModel; }
        virtual Artworks::IMetadataOperator *getMetadataOperator() override { return &m_BasicModel; }

    protected:
        virtual void submitForInspection() override;
        virtual void doJustEdited() override;

        // DelayedActionEntity implementation
    protected:
        virtual void doKillTimer(int timerId) override { this->killTimer(timerId); }
        virtual int doStartTimer(int interval, Qt::TimerType timerType) override { return this->startTimer(interval, timerType); }
        virtual void doOnTimer() override;
        virtual void timerEvent(QTimerEvent *event) override { onQtTimer(event); }
        virtual void callBaseTimer(QTimerEvent *event) override { QObject::timerEvent(event); }

    private:
        Common::Hold m_HoldPlaceholder;
        Artworks::BasicMetadataModel m_BasicModel;
        SpellCheck::SpellCheckItemInfo m_SpellCheckInfo;
        CurrentEditableModel &m_CurrentEditableModel;
        Commands::ICommandManager &m_CommandManager;
    };
}

#endif // QUICKBUFFER_H

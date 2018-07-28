/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SPELLCHECKERSERVICE_H
#define SPELLCHECKERSERVICE_H

#include <QObject>
#include <QString>
#include <QVector>
#include <vector>
#include <deque>
#include <Artworks/basickeywordsmodel.h>
#include <Common/flags.h>
#include <Common/isystemenvironment.h>
#include <Common/types.h>
#include <Common/messages.h>
#include "spellcheckworker.h"
#include "userdictionary.h"

namespace Artworks {
    class ArtworkMetadata;
    class ArtworksSnapshot;
}

namespace Warnings {
    class WarningsService;
}

namespace Models {
    class SettingsModel;
}

namespace SpellCheck {
    class SpellCheckService:
            public QObject,
            public Common::MessagesTarget<Common::NamedType<Artworks::ArtworkMetadata*, Common::MessageType::SpellCheck>>,
            public Common::MessagesTarget<Common::NamedType<Artworks::BasicKeywordsModel*, Common::MessageType::SpellCheck>>
    {
        Q_OBJECT
        Q_PROPERTY(int userDictWordsNumber READ getUserDictWordsNumber NOTIFY userDictWordsNumberChanged)

    public:
        SpellCheckService(Common::ISystemEnvironment &environment,
                            Warnings::WarningsService &warningsService,
                            Models::SettingsModel &settingsModel);

    public:
        virtual void handleMessage(const Common::NamedType<Artworks::ArtworkMetadata*, Common::MessageType::SpellCheck> &event) override;
        virtual void handleMessage(const Common::NamedType<Artworks::BasicKeywordsModel*, Common::MessageType::SpellCheck> &event) override;

    public:
        void startService(Helpers::AsyncCoordinator &initCoordinator);
        void stopService();

        bool isAvailable() const { return true; }
        bool isBusy() const;

        void submitItem(Artworks::BasicKeywordsModel *itemToCheck, Common::SpellCheckFlags flags);
        void submitArtworks(const Artworks::ArtworksSnapshot &snapshot);
        void submitArtwork(Artworks::ArtworkMetadata *artwork);
        SpellCheckWorker::batch_id_t submitItems(const std::vector<Artworks::ArtworkMetadata *> &itemsToCheck);
        SpellCheckWorker::batch_id_t submitItems(const std::vector<Artworks::BasicKeywordsModel *> &itemsToCheck);
        void submitItems(const std::vector<Artworks::ArtworkMetadata *> &itemsToCheck, const QStringList &wordsToCheck);
        virtual QStringList suggestCorrections(const QString &word) const;
        int getUserDictWordsNumber();

#ifdef INTEGRATION_TESTS
    public:
        int getSuggestionsCount();
#endif

    public:
        QStringList getUserDictionary();
        void updateUserDictionary(const QStringList &words);

    public:
        Q_INVOKABLE void cancelCurrentBatch();
        Q_INVOKABLE bool hasAnyPending();
        Q_INVOKABLE void addWordToUserDictionary(const QString &word);
        Q_INVOKABLE void clearUserDictionary();

    signals:
        void cancelSpellChecking();
        void spellCheckQueueIsEmpty();
        void serviceAvailable();
        void userDictWordsNumberChanged();

    private slots:
        void workerFinished();
        void workerDestroyed(QObject *object);

    private:
        Common::WordAnalysisFlags getWordAnalysisFlags() const;
        QString getDictsRoot() const;

    private:
        Common::ISystemEnvironment &m_Environment;
        Warnings::WarningsService &m_WarningsService;
        SpellCheckWorker *m_SpellCheckWorker;
        UserDictionary m_UserDictionary;
        Models::SettingsModel &m_SettingsModel;
        QString m_DictionariesPath;
        volatile bool m_IsStopped;
    };
}

#endif // SPELLCHECKERSERVICE_H

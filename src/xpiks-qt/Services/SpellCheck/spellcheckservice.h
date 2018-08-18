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
#include <functional>
#include <Common/messages.h>
#include "spellcheckworker.h"
#include "userdictionary.h"
#include "ispellcheckservice.h"
#include <Artworks/basickeywordsmodel.h>
#include <Common/flags.h>
#include <Common/isystemenvironment.h>
#include <Common/types.h>

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
            public ISpellCheckService
    {
        Q_OBJECT
        Q_PROPERTY(int userDictWordsNumber READ getUserDictWordsNumber NOTIFY userDictWordsNumberChanged)

    public:
        SpellCheckService(Common::ISystemEnvironment &environment,
                          Common::IFlagsProvider<Common::WordAnalysisFlags> &analysisFlagsProvider);
        virtual ~SpellCheckService();

    public:
        void startService(Helpers::AsyncCoordinator &initCoordinator, Warnings::WarningsService &warningsService);
        void stopService();

        // ISpellCheckService interface
    private:
        virtual quint32 submitItems(const std::vector<std::shared_ptr<Artworks::IBasicModelSource> > &items,
                                    const QStringList &wordsToCheck = QStringList()) override;
        virtual void submitItem(const std::shared_ptr<Artworks::IBasicModelSource> &item) override;

    public:
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
        QString getDictsRoot() const;

    private:
        Common::ISystemEnvironment &m_Environment;
        SpellCheckWorker *m_SpellCheckWorker;
        UserDictionary m_UserDictionary;
        Common::IFlagsProvider<Common::WordAnalysisFlags> &m_AnalysisFlagsProvider;
        QString m_DictionariesPath;
        volatile bool m_IsStopped;
    };
}

#endif // SPELLCHECKERSERVICE_H

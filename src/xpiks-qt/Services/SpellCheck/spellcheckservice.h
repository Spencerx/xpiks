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
    class UserDictionary;

    class SpellCheckService:
            public QObject,
            public ISpellCheckService
    {
        Q_OBJECT

    public:
        SpellCheckService(Common::ISystemEnvironment &environment,
                          Common::IFlagsProvider<Common::WordAnalysisFlags> &analysisFlagsProvider);
        virtual ~SpellCheckService();

    public:
        void startService(Helpers::AsyncCoordinator &initCoordinator,
                          UserDictionary &userDictionary,
                          Warnings::WarningsService &warningsService);
        void stopService();

#ifdef INTEGRATION_TESTS
    public:
        bool isBusy() const;
#endif

        // ISpellCheckService interface
    private:
        virtual quint32 submitItems(const std::vector<std::shared_ptr<Artworks::IBasicModelSource> > &items,
                                    Common::SpellCheckFlags flags) override;
        virtual void submitItem(const std::shared_ptr<Artworks::IBasicModelSource> &item,
                                Common::SpellCheckFlags flags) override;

    public:
        virtual QStringList suggestCorrections(const QString &word) const;
        int getUserDictWordsNumber();

#ifdef INTEGRATION_TESTS
    public:
        int getSuggestionsCount();
        void clearSuggestions();
#endif

    public:
        Q_INVOKABLE void cancelCurrentBatch();
        Q_INVOKABLE bool hasAnyPending();

    signals:
        void cancelSpellChecking();
        void spellCheckQueueIsEmpty();
        void serviceAvailable();

    private slots:
        void workerFinished();
        void workerDestroyed(QObject *object);

    private:
        QString getDictsRoot() const;

    private:
        Common::ISystemEnvironment &m_Environment;
        SpellCheckWorker *m_SpellCheckWorker;
        Common::IFlagsProvider<Common::WordAnalysisFlags> &m_AnalysisFlagsProvider;
        QString m_DictionariesPath;
        volatile bool m_IsStopped;
    };
}

#endif // SPELLCHECKERSERVICE_H

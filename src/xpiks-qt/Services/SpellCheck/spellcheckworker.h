/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SPELLCHECKWORKER_H
#define SPELLCHECKWORKER_H

#include <QString>
#include <QStringList>
#include <QReadWriteLock>
#include <QSet>
#include "spellcheckitem.h"
#include <Common/itemprocessingworker.h>
#include <Common/isystemenvironment.h>
#include <Common/lrucache.h>
#include <Models/settingsmodel.h>
#include <Helpers/asynccoordinator.h>
#include <Helpers/hashhelpers.h>

class Hunspell;
class QTextCodec;

namespace Warnings {
    class WarningsService;
}

namespace Artworks {
    class ArtworkMetadata;
}

namespace SpellCheck {
    class UserDictionary;

    class SpellCheckWorker:
            public QObject,
            public Common::ItemProcessingWorker<SpellCheckItem, Artworks::ArtworkMetadata>
    {
        Q_OBJECT

    public:
        SpellCheckWorker(const QString &dictsRoot,
                         Common::ISystemEnvironment &environment,
                         UserDictionary &userDictionary,
                         Warnings::WarningsService &warningsService,
                         Helpers::AsyncCoordinator &initCoordinator,
                         QObject *parent=0);
        virtual ~SpellCheckWorker();

    public:
        QStringList getUserDictionary() const;
        QStringList retrieveCorrections(const QString &word);
        int getUserDictionarySize() const;

    protected:
        virtual bool initWorker() override;        
        virtual std::shared_ptr<Artworks::ArtworkMetadata> processWorkItem(WorkItem &workItem) override;
        void processSpellingQuery(std::shared_ptr<SpellCheckItem> &item);

    protected:
        virtual void onQueueIsEmpty() override {
            /* Notify on emptiness only for batches with separator */
            /* emit queueIsEmpty(); */
        }
        virtual void onResultsAvailable(std::vector<WorkResult> &results) override;
        virtual void onWorkerStopped() override { emit stopped(); }

    public slots:
        void process() { doWork(); }
        void cancel() { stopWorking(); }

    signals:
        void stopped();
        void queueIsEmpty();

#if defined(INTEGRATION_TESTS) || defined(UI_TESTS)
    public:
        int getSuggestionsCount() const { return (int)m_Suggestions.size(); }
        void clearSuggestions();
#endif

    private:
        void detectAffEncoding();
        QStringList suggestCorrections(const QString &word);
        bool checkQuerySpelling(const std::shared_ptr<SpellCheckQueryItem> &queryItem);
        bool checkWordSpelling(const QString &word);
        void stemWord(const std::shared_ptr<SpellCheckQueryItem> &queryItem);
        QString getWordStem(const QString &word);
        bool isHunspellSpellingCorrect(const QString &word) const;
        void findSemanticDuplicates(const std::vector<std::shared_ptr<SpellCheckQueryItem> > &queries);
        void findSuggestions(const QString &word);

    private:
        Common::ISystemEnvironment &m_Environment;
        Helpers::AsyncCoordinator &m_InitCoordinator;
        UserDictionary &m_UserDictionary;
        Warnings::WarningsService &m_WarningsService;
        Common::LRUCache<QString, QStringList> m_Suggestions;
        QSet<QString> m_WrongWords;
        QReadWriteLock m_SuggestionsLock;
        QString m_DictsRoot;
        QString m_Encoding;
        Hunspell *m_Hunspell;
        // Coded does not need destruction
        QTextCodec *m_Codec;
    };
}

#endif // SPELLCHECKWORKER_H

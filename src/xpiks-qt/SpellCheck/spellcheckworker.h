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
#include <QHash>
#include <QSet>
#include "../Common/itemprocessingworker.h"
#include "../Models/settingsmodel.h"
#include "spellcheckitem.h"
#include "../Helpers/asynccoordinator.h"
#include "../Common/isystemenvironment.h"

class Hunspell;
class QTextCodec;

namespace SpellCheck {
    class UserDictionary;

    class SpellCheckWorker: public QObject, public Common::ItemProcessingWorker<ISpellCheckItem>
    {
        Q_OBJECT

    public:
        SpellCheckWorker(const QString &dictsRoot,
                         Common::ISystemEnvironment &environment,
                         UserDictionary &userDictionary,
                         Helpers::AsyncCoordinator *initCoordinator,
                         QObject *parent=0);
        virtual ~SpellCheckWorker();

    public:
        const QStringList &getUserDictionary() const { return m_UserDictionary.getWords(); }
        QStringList retrieveCorrections(const QString &word);
        int getUserDictionarySize() const { return m_UserDictionary.size(); }

    protected:
        virtual bool initWorker() override;        
        virtual std::shared_ptr<ResultType> processWorkItem(WorkItem &workItem) override;
        //virtual void processOneItem(std::shared_ptr<ISpellCheckItem> &item) override;

    private:
        void processQueryItem(std::shared_ptr<SpellCheckItem> &item);
        void processChangeUserDict(std::shared_ptr<ModifyUserDictItem> &item);

    protected:
        virtual void workerStopped() override { emit stopped(); }

    public slots:
        void process() { doWork(); }
        void cancel() { stopWorking(); }

    signals:
        void stopped();
        void queueIsEmpty();
        void wordsNumberChanged(int number);
        void userDictUpdate(const QStringList &keywords, bool overwritten);
        void userDictCleared();

#ifdef INTEGRATION_TESTS
    public:
        int getSuggestionsCount() const { return m_Suggestions.count(); }
#endif

    private:
        void detectAffEncoding();
        QStringList suggestCorrections(const QString &word);
        bool checkWordSpelling(const std::shared_ptr<SpellCheckQueryItem> &queryItem);
        bool checkWordSpelling(const QString &word);
        void stemWord(const std::shared_ptr<SpellCheckQueryItem> &queryItem);
        QString getWordStem(const QString &word);
        bool isHunspellSpellingCorrect(const QString &word) const;
        void findSemanticDuplicates(const std::vector<std::shared_ptr<SpellCheckQueryItem> > &queries);
        void findSuggestions(const QString &word);
        void initUserDictionary();
        void cleanUserDict();
        void changeUserDict(const QStringList &words, bool overwrite);
        void signalUserDictWordsCount();

    private:
        Common::ISystemEnvironment &m_Environment;
        Helpers::AsyncCoordinator *m_InitCoordinator;
        QHash<QString, QStringList> m_Suggestions;
        QSet<QString> m_WrongWords;
        UserDictionary &m_UserDictionary;
        QReadWriteLock m_SuggestionsLock;
        QString m_DictsRoot;
        QString m_Encoding;
        Hunspell *m_Hunspell;
        // Coded does not need destruction
        QTextCodec *m_Codec;
    };
}

#endif // SPELLCHECKWORKER_H

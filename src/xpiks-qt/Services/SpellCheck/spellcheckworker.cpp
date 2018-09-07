/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "spellcheckworker.h"

#include <QFile>
#include <QTextStream>
#include <QTextCodec>
#include <QDir>
#include <QUrl>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QThread>
#include <hunspell/hunspell.hxx>
#include "spellcheckitem.h"
#include <Common/defines.h>
#include <Common/flags.h>
#include <Helpers/stringhelper.h>
#include <Artworks/artworkmetadata.h>
#include <Artworks/artworkssnapshot.h>
#include <Services/Warnings/warningsservice.h>
#include <Helpers/cpphelpers.h>
#include "userdictionary.h"

#define EN_HUNSPELL_DIC "en_US.dic"
#define EN_HUNSPELL_AFF "en_US.aff"

#define MINIMUM_LENGTH_FOR_STEMMING 3
#define SPELLCHECK_WORKER_SLEEP_DELAY 500
#define SPELLCHECK_DELAY_PERIOD 50

#if defined(INTEGRATION_TESTS) || defined(CORE_TESTS)
    #define SUGGESTIONS_CACHE_SIZE 10
    #define SUGGESTIONS_CACHE_ELASTICITY 2
#else
    #define SUGGESTIONS_CACHE_SIZE 1000
    #define SUGGESTIONS_CACHE_ELASTICITY 200
#endif

namespace SpellCheck {
    SpellCheckWorker::SpellCheckWorker(const QString &dictsRoot,
                                       Common::ISystemEnvironment &environment,
                                       UserDictionary &userDictionary,
                                       Warnings::WarningsService &warningsService,
                                       Helpers::AsyncCoordinator &initCoordinator,
                                       QObject *parent):
        QObject(parent),
        ItemProcessingWorker(SPELLCHECK_DELAY_PERIOD),
        m_Environment(environment),
        m_InitCoordinator(initCoordinator),
        m_UserDictionary(userDictionary),
        m_WarningsService(warningsService),
        m_Suggestions(SUGGESTIONS_CACHE_SIZE, SUGGESTIONS_CACHE_ELASTICITY),
        m_DictsRoot(dictsRoot),
        m_Hunspell(NULL),
        m_Codec(NULL)
    {
        Q_ASSERT(!dictsRoot.isEmpty());
    }

    SpellCheckWorker::~SpellCheckWorker() {
        if (m_Hunspell != NULL) {
            delete m_Hunspell;
        }

        LOG_INFO << "destroyed";
    }

    QStringList SpellCheckWorker::getUserDictionary() const { return m_UserDictionary.getWords(); }

    bool SpellCheckWorker::initWorker() {
        LOG_INFO << "Dicts root:" << m_DictsRoot;

        Helpers::AsyncCoordinatorUnlocker unlocker(m_InitCoordinator);
        Q_UNUSED(unlocker);

        QDir resourcesDir(m_DictsRoot);
        QString affPath = resourcesDir.absoluteFilePath(EN_HUNSPELL_AFF);
        QString dicPath = resourcesDir.absoluteFilePath(EN_HUNSPELL_DIC);

        bool initResult = false;

        if (QFileInfo(affPath).exists() && QFileInfo(dicPath).exists()) {
#ifdef Q_OS_WIN
            // specific Hunspell handling of UTF-8 encoded pathes
            affPath = "\\\\?\\" + QDir::toNativeSeparators(affPath);
            dicPath = "\\\\?\\" + QDir::toNativeSeparators(dicPath);
#endif

            try {
                m_Hunspell = new Hunspell(affPath.toUtf8().constData(),
                                          dicPath.toUtf8().constData());
                LOG_DEBUG << "Hunspell initialized with AFF" << affPath << "and DIC" << dicPath;
                initResult = true;
                m_Encoding = QString::fromLatin1(m_Hunspell->get_dic_encoding());
                m_Codec = QTextCodec::codecForName(m_Encoding.toLatin1().constData());
            } catch (...) {
                LOG_DEBUG << "Error in Hunspell with AFF" << affPath << "and DIC" << dicPath;
                m_Hunspell = NULL;
            }
        } else {
            LOG_WARNING << "DIC or AFF file not found." << dicPath << "||" << affPath;
        }

        return initResult;
    }

    std::shared_ptr<Artworks::ArtworkMetadata> SpellCheckWorker::processWorkItem(WorkItem &workItem) {
        std::shared_ptr<Artworks::ArtworkMetadata> result;
        if (workItem.isSeparator()) {
            emit queueIsEmpty();
        } else {
            this->processSpellingQuery(workItem.m_Item);

            if (workItem.isMilestone()) {
                QThread::msleep(SPELLCHECK_WORKER_SLEEP_DELAY);
            }

            result = std::dynamic_pointer_cast<Artworks::ArtworkMetadata>(workItem.m_Item->getBasicModelSource());
        }

        return result;
    }

    void SpellCheckWorker::processSpellingQuery(std::shared_ptr<SpellCheckItem> &item) {
        const bool neededSuggestions = item->needsSuggestions();
        auto &queryItems = item->getQueries();
        const auto wordAnalysisFlags = item->getWordAnalysisFlags();
        const bool shouldCheckSpelling = Common::HasFlag(wordAnalysisFlags, Common::WordAnalysisFlags::Spelling);
        const bool shouldStemWord = Common::HasFlag(wordAnalysisFlags, Common::WordAnalysisFlags::Stemming);
        bool anyWrong = false;

        if (!neededSuggestions) {
            const size_t size = queryItems.size();
            Q_ASSERT(size > 0);
            for (size_t i = 0; i < size; ++i) {
                auto &queryItem = queryItems.at(i);
                bool isOk = true;

                if (shouldCheckSpelling) {
                    isOk = checkQuerySpelling(queryItem);
                }

                if (shouldStemWord) {
                    stemWord(queryItem);
                }

                anyWrong = anyWrong || !isOk;
            }

            if (shouldStemWord && !item->getIsOnlyOneKeyword()) {
                findSemanticDuplicates(queryItems);
            }

            item->accountResults();
            item->submitSpellCheckResult();
        } else {
            for (auto &queryItem: queryItems) {
                if (!queryItem->m_IsCorrect) {
                    findSuggestions(queryItem->m_Word);
                }
            }
        }

        if (anyWrong) {
            item->requestSuggestions();
            this->submitItem(item);
        }
    }

    void SpellCheckWorker::onResultsAvailable(std::vector<WorkResult> &results) {
        auto rawSnapshot = Helpers::map<WorkResult, std::shared_ptr<Artworks::ArtworkMetadata>>(
                    results,
                    [](const WorkResult &result) { return result.m_Result; });
        m_WarningsService.submitItems(
                    Artworks::ArtworksSnapshot(rawSnapshot));
    }

#if defined(INTEGRATION_TESTS) || defined(UI_TESTS)
    void SpellCheckWorker::clearSuggestions() {
        QWriteLocker locker(&m_SuggestionsLock);
        Q_UNUSED(locker);
        m_Suggestions.clear();
    }
#endif

    QStringList SpellCheckWorker::retrieveCorrections(const QString &word) {
        QReadLocker locker(&m_SuggestionsLock);
        Q_UNUSED(locker);
        QStringList result;

        if (!m_Suggestions.tryGet(word, result)) {
            LOG_VERBOSE << "Suggestion not found for:" << word;
        }

        return result;
    }

    int SpellCheckWorker::getUserDictionarySize() const { return m_UserDictionary.size(); }

    QStringList SpellCheckWorker::suggestCorrections(const QString &word) {
        QStringList suggestions;
        std::vector<std::string> suggestWordList;

        try {
            // Encode from Unicode to the encoding used by current dictionary
            std::string encodedWord = m_Codec->fromUnicode(word).toStdString();
            suggestWordList = m_Hunspell->suggest(encodedWord);
            LOG_VERBOSE << "Found" << suggestWordList.size() << "suggestions for" << word;
            QString lowerWord = word.toLower();

            for (size_t i = 0; i < suggestWordList.size(); ++i) {
                QString suggestion = m_Codec->toUnicode(QByteArray::fromStdString(suggestWordList[i]));

                if (suggestion.toLower() != lowerWord) {
                    suggestions << suggestion;
                }
            }
        } catch (...) {
            LOG_WARNING << "Error for keyword:" << word;
        }
        return suggestions;
    }

    bool SpellCheckWorker::checkQuerySpelling(const std::shared_ptr<SpellCheckQueryItem> &queryItem) {
        bool isOk = false;

        const QString &word = queryItem->m_Word;
        const bool isInUserDict = m_UserDictionary.contains(word);

        isOk = isInUserDict || checkWordSpelling(word);
        queryItem->m_IsCorrect = isOk;

        return isOk;
    }

    bool SpellCheckWorker::checkWordSpelling(const QString &word) {
        bool isOk = false;

        const bool isCached = m_WrongWords.contains(word);

        if (!isCached) {
            isOk = isHunspellSpellingCorrect(word);

            if (!isOk) {
                QString capitalized = word;
                capitalized[0] = capitalized[0].toUpper();

                if (isHunspellSpellingCorrect(capitalized)) {
                    isOk = true;
                }
            }
        }

        if (!isOk) {
            m_WrongWords.insert(word);
        }

        return isOk;
    }

    QString SpellCheckWorker::getWordStem(const QString &word) {
        QString result;

        if (word.isEmpty()) { return result; }

        std::string encodedWord = m_Codec->fromUnicode(word).toStdString();
        std::vector<std::string> stems;

        try {
            stems = m_Hunspell->stem(encodedWord);
        } catch(...) {
            return result;
        }

#ifdef INTEGRATION_TESTS
        QStringList stemsList;
        for (auto &stm: stems) { stemsList.append(QString::fromStdString(stm)); }
        LOG_DEBUG << "Stemming:" << word << "->" << stemsList;
#endif

        const size_t size = stems.size();
        if (size > 0) {
            size_t minIndex = 0, minSize = stems[0].length();

            for (size_t i = 1; i < size; i++) {
                const size_t currSize = stems[i].length();
                if (currSize < minSize) {
                    minSize = currSize;
                    minIndex = i;
                }
            }

            result = QString::fromStdString(stems[minIndex]);
        } else {
            // this may help to find at least exact duplicates
            // for words unknown to Hunspell
            result = word.toLower();
        }

        return result;
    }

    void SpellCheckWorker::stemWord(const std::shared_ptr<SpellCheckQueryItem> &queryItem) {
        QString word = queryItem->m_Word;
        if (word.length() >= MINIMUM_LENGTH_FOR_STEMMING) {
            queryItem->m_Stem = getWordStem(word.toLower());
        }
    }

    bool SpellCheckWorker::isHunspellSpellingCorrect(const QString &word) const {
        bool isOk = false;

        try {
            std::string encodedWord = m_Codec->fromUnicode(word).toStdString();
            isOk = m_Hunspell->spell(encodedWord) != 0;
        } catch (...) {
            isOk = false;
        }
        return isOk;
    }

    void SpellCheckWorker::findSemanticDuplicates(const std::vector<std::shared_ptr<SpellCheckQueryItem> > &queries) {
        LOG_VERBOSE_OR_DEBUG << "#";
        const size_t size = queries.size();

        QHash<QString, QVector<size_t> > stemToIndexMap;

        for (size_t i = 0; i < size; ++i) {
            auto &queryItem = queries.at(i);
            if (queryItem->m_Stem.isEmpty()) {
                LOG_WARNING << "Stem is empty for" << queryItem->m_Word;
                continue;
            }

            if (queryItem->m_Stem.length() <= 2) { continue; }

            stemToIndexMap[queryItem->m_Stem].append(i);
        }

        LOG_VERBOSE_OR_DEBUG << "Stems hash has" << stemToIndexMap.size() << "item(s)";
        LOG_VERBOSE << stemToIndexMap;

        auto itEnd = stemToIndexMap.constEnd();
        auto it = stemToIndexMap.constBegin();

        while (it != itEnd) {
            const QVector<size_t> &sameStemIndices = it.value();
            if (sameStemIndices.size() > 1) {
                const int size = sameStemIndices.size();

                for (int i = 0; i < size; i++) {
                    for (int j = 0; j < size; j++) {
                        if (i == j) { continue; }

                        size_t index1 = sameStemIndices[i];
                        size_t index2 = sameStemIndices[j];

                        auto &query1 = queries[index1];
                        auto &query2 = queries[index2];
                        Q_ASSERT(query1->m_Stem == query2->m_Stem);

                        if (query1->m_IsDuplicate && query2->m_IsDuplicate) { continue; }

                        if (Helpers::areSemanticDuplicates(query1->m_Word, query2->m_Word)) {
                            LOG_VERBOSE_OR_DEBUG << "detected as duplicates:" << "[" << query1->m_Index << "]:"
                                                 << query1->m_Word << "[" << query2->m_Index << "]:" << query2->m_Word;
                            query1->m_IsDuplicate = true;
                            query2->m_IsDuplicate = true;
                        }
                    }
                }
            }

            ++it;
        }
    }

    void SpellCheckWorker::findSuggestions(const QString &word) {
        LOG_VERBOSE << word;
        bool needsCorrections = false;
        QStringList suggestions;

        m_SuggestionsLock.lockForRead();
        needsCorrections = !m_Suggestions.tryGet(word, suggestions);
        m_SuggestionsLock.unlock();

        if (needsCorrections) {
            suggestions = suggestCorrections(word);
            m_SuggestionsLock.lockForWrite();
            {
                m_Suggestions.put(word, suggestions);
            }
            m_SuggestionsLock.unlock();
        }
    }
}

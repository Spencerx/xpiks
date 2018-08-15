/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "translationmanager.h"
#include <QDir>
#include <QSet>
#include <QUrl>
#include <QFile>
#include <QTextStream>
#include <QDirIterator>
#include <Common/defines.h>
#include "translationquery.h"
#include "translationservice.h"
#include <Models/settingsmodel.h>
#include <Helpers/constants.h>
#include <Helpers/asynccoordinator.h>

#define DEFAULT_SELECTED_DICT_INDEX -1

#define SHORT_TRANSLATION_SYMBOLS 500
#define BOOKNAME QLatin1String("bookname")

namespace Translation {
    bool parseIfoFile(const QString &fullIfoPath, DictionaryInfo &info) {
        LOG_INFO << fullIfoPath;
        QFile file(fullIfoPath);
        bool success = false;

        if (file.open(QIODevice::ReadOnly)) {
            QTextStream in(&file);

            QString line;

            do {
                if (!in.atEnd()) {
                    line = in.readLine();
                    if ((line != "StarDict's treedict ifo file") &&
                            (line != "StarDict's dict ifo file")) {
                        LOG_WARNING << "Wrong header" << line;
                        break;
                    }
                }

                while (!in.atEnd()) {
                    line = in.readLine();
                    if (line.startsWith(BOOKNAME)) {
                        info.m_Description = line.mid(BOOKNAME.size() + 1).simplified();
                        info.m_FullIfoPath = fullIfoPath;
                        success = true;
                        break;
                    }
                }
            } while (false);
        }

        return success;
    }

    TranslationManager::TranslationManager(Common::ISystemEnvironment &environment,
                                           TranslationService &translationService,
                                           QObject *parent) :
        QObject(parent),
        m_Environment(environment),
        m_TranslationService(translationService),
        m_State("translator", environment),
        m_AllowedSuffixes({"idx", "idx.dz", "idx.oft", "dict.dz", "dict", "ifo"}),
        m_SelectedDictionaryIndex(-1),
        m_IsBusy(false),
        m_HasMore(false)
    {
        m_TranslateTimer.setSingleShot(true);
        QObject::connect(&m_TranslateTimer, &QTimer::timeout, this, &TranslationManager::updateTranslationTimer);

        QObject::connect(&m_TranslationService, &TranslationService::translationAvailable,
                         this, &TranslationManager::translationArrived);
    }

    void TranslationManager::setQuery(const QString &value) {
        bool anySignificantDifference = value.simplified() != m_Query;

        if (m_Query != value) {
            m_Query = value;
            emit queryChanged();
        }

        if (anySignificantDifference) {
            m_TranslateTimer.start(1000);
        }
    }

    void TranslationManager::setSelectedDictionaryIndex(int value) {
        LOG_INFO << value;
        LOG_INTEGRATION_TESTS << "Current index" << m_SelectedDictionaryIndex;
        LOG_INTEGRATION_TESTS << "Overall" << m_DictionariesList.length() << "dictionaries";

        if (m_SelectedDictionaryIndex != value) {
            LOG_INFO << "Changing current dictionary";
            m_SelectedDictionaryIndex = value;
            emit selectedDictionaryIndexChanged();

            if ((0 <= value) && (value < m_DictionariesList.length())) {
                auto &dictInfo = m_DictionariesList[value];
                LOG_INFO << "Selecting" << dictInfo.m_Description;
                m_TranslationService.selectDictionary(dictInfo.m_FullIfoPath);
            } else {
                LOG_WARNING << "Cannot select dictionary path: indices to not match";
            }

            if (!m_Query.isEmpty()) {
                m_TranslateTimer.start(1000);
                setIsBusy(true);
            }

            m_State.setValue(Constants::translatorSelectedDictIndex, value);
            m_State.sync();
        }
    }

    QStringList TranslationManager::getDictionariesDescriptions() const {
        QStringList descriptions;
        descriptions.reserve(m_DictionariesList.length());

        foreach (const DictionaryInfo &dict, m_DictionariesList) {
            descriptions.append(dict.m_Description);
        }

        return descriptions;
    }

    void TranslationManager::doInitializeDictionaries() {
        m_Environment.ensureDirExists(Constants::TRANSLATOR_DIR);
        m_DictionariesDirPath = m_Environment.path({Constants::TRANSLATOR_DIR});

        QDirIterator it(m_DictionariesDirPath, QStringList() << "*.ifo", QDir::Files, QDirIterator::NoIteratorFlags);

        while (it.hasNext()) {
            QString ifoFullPath = it.next();

            DictionaryInfo di;
            if (parseIfoFile(ifoFullPath, di) &&
                    hasAllNeededComponents(ifoFullPath)) {
                m_DictionariesList.append(di);
                LOG_INFO << "Parsed" << di.m_Description;
            } else {
                LOG_WARNING << "Failed to parse IFO:" << ifoFullPath;
            }
        }
    }

    bool TranslationManager::acquireDictionary(const QString &anyDictFilePath) {
        QFileInfo fi(anyDictFilePath);
        Q_ASSERT(fi.exists());
        QString longPrefix = QDir::cleanPath(fi.absolutePath() + QChar('/') + fi.baseName());
        QString shortPrefix = fi.baseName();

        bool anyError = false, anyFileCopied = false,
                indexFound = false, dictFound = false, ifoFound = false;

        foreach (const QString &suffix, m_AllowedSuffixes) {
            QString probablePath = longPrefix + "." + suffix;

            if (QFileInfo(probablePath).exists()) {
                LOG_INFO << "File found:" << probablePath;
                QString localDict = QDir::cleanPath(m_DictionariesDirPath + QChar('/') + shortPrefix + "." + suffix);

                if (QFile::copy(probablePath, localDict)) {
                    LOG_INFO << "Copied to" << localDict;
                    anyFileCopied = true;

                    // TODO: refactor this someday to check just needed
                    // extensions instead of looping all of them
                    indexFound = indexFound || suffix.contains("idx");
                    dictFound = dictFound || suffix.contains("dict");
                    ifoFound = ifoFound || suffix.contains("ifo");
                } else {
                    LOG_WARNING << "Failed to copy as:" << localDict;
                    anyError = true;
                    break;
                }
            } else {
                LOG_INFO << "File NOT found:" << probablePath;
            }
        }

        LOG_INFO << "IFO found" << ifoFound << "Index found" << indexFound << "Dictionary found" << dictFound;

        bool success = (anyFileCopied) && (!anyError) && (ifoFound && dictFound && indexFound);
        return success;
    }

    bool TranslationManager::hasAllNeededComponents(const QString &anyDictFilePath) const {
        QFileInfo fi(anyDictFilePath);
        Q_ASSERT(fi.exists());

        QString longPrefix = QDir::cleanPath(fi.absolutePath() + QChar('/') + fi.baseName());
        bool indexFound = false, dictFound = false, ifoFound = false;

        foreach (const QString &suffix, m_AllowedSuffixes) {
            QString probablePath = longPrefix + "." + suffix;

            if (QFileInfo(probablePath).exists()) {
                // TODO: refactor this someday to check just needed
                // extensions instead of looping all of them
                indexFound = indexFound || suffix.contains("idx");
                dictFound = dictFound || suffix.contains("dict");
                ifoFound = ifoFound || suffix.contains("ifo");
            }
        }

        bool success = ifoFound && dictFound && indexFound;
        return success;
    }

    void TranslationManager::clear() {
        m_Query.clear();
        m_FullTranslation.clear();
        m_ShortenedTranslation.clear();
        m_HasMore = false;
        emit queryChanged();
        emit fullTranslationChanged();
        emit shortTranslationChanged();
        emit hasMoreChanged();
    }

    bool TranslationManager::addDictionary(const QUrl &url) {
        Q_ASSERT(url.isLocalFile());
        QString anyDictFilePath = url.toLocalFile();
        LOG_INFO << anyDictFilePath;
        QFileInfo fi(anyDictFilePath);
        bool success = false;

        QSet<QString> allowedSuffixes = m_AllowedSuffixes.toSet();

        do {
            if (!fi.exists()) {
                LOG_WARNING << "File does not exist:" << anyDictFilePath;
                break;
            }

            if (!fi.isFile()) {
                LOG_WARNING << "Path is not a file:" << anyDictFilePath;
                break;
            }

            if (!allowedSuffixes.contains(fi.completeSuffix().toLower())) {
                LOG_WARNING << "File" << fi.completeSuffix() << "is unsupported";
                break;
            }

            QString probableIfoPath = QDir::cleanPath(fi.absolutePath() + QChar('/') +
                                                      fi.baseName() + ".ifo");
            DictionaryInfo di;
            if (parseIfoFile(probableIfoPath, di)) {
                LOG_INFO << "Parsed" << di.m_Description;
                if (acquireDictionary(probableIfoPath)) {
                    m_DictionariesList.append(di);
                    success = true;
                } else {
                    LOG_WARNING << "Failed to acquire dictionary" << probableIfoPath;
                }
            } else {
                LOG_WARNING << "Failed to parse IFO:" << probableIfoPath;
            }
        } while (false);

        if (success && (m_SelectedDictionaryIndex == -1)) {
            setSelectedDictionaryIndex(0);
        }

        emit dictionariesChanged();

        return success;
    }

    void TranslationManager::translationArrived() {
        LOG_DEBUG << "#";
        QString translation;

        if (m_TranslationService.retrieveTranslation(m_Query, translation)) {
            m_FullTranslation = translation;
            m_ShortenedTranslation = translation.left(SHORT_TRANSLATION_SYMBOLS);
        } else {
            QString nothingFound = QObject::tr("No results");

            m_FullTranslation = nothingFound;
            m_ShortenedTranslation = nothingFound;
        }

        m_HasMore = m_FullTranslation.length() != m_ShortenedTranslation.length();
        emit hasMoreChanged();

        if (m_HasMore) {
            m_ShortenedTranslation.append("...");
        }

        emit fullTranslationChanged();
        emit shortTranslationChanged();

        setIsBusy(false);
    }

    void TranslationManager::updateTranslationTimer() {
        LOG_DEBUG << "#";
        m_TranslationService.translate(m_Query);
        setIsBusy(true);
    }

    void TranslationManager::initializationFinished() {
        LOG_DEBUG << "#";

        m_State.init();

        const int selectedDictIndex = m_State.getInt(Constants::translatorSelectedDictIndex, DEFAULT_SELECTED_DICT_INDEX);
        if ((0 <= selectedDictIndex) && (selectedDictIndex < m_DictionariesList.length())) {
            setSelectedDictionaryIndex(selectedDictIndex);
        } else {
            if (!m_DictionariesList.empty()) {
                setSelectedDictionaryIndex(0);
            }
        }

        emit dictionariesChanged();
        emit selectedDictionaryIndexChanged();
    }
}

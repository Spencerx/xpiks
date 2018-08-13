/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "userdictionary.h"
#include <QFile>
#include <Common/logging.h>
#include <Helpers/constants.h>
#include <Helpers/stringhelper.h>

namespace SpellCheck {
    UserDictionary::UserDictionary(Common::ISystemEnvironment &environment):
        m_Filepath(environment.path({Constants::USER_DICT_FILENAME}))
    {
    }

    void UserDictionary::initialize() {
        LOG_INFO << m_Filepath;
        QFile userDictonaryFile(m_Filepath);

        if (userDictonaryFile.open(QIODevice::ReadOnly)) {
            QTextStream stream(&userDictonaryFile);
            for (QString word = stream.readLine(); !word.isEmpty(); word = stream.readLine()) {
                addWord(word);
            }
        } else {
            LOG_WARNING << "Cannot open" << m_Filepath;
        }

        LOG_INFO << "User Dictionary contains:" << this->size() << "item(s)";
    }

    void UserDictionary::save() {
        LOG_DEBUG << "#";
        QFile userDictonaryFile(m_Filepath);
        if (userDictonaryFile.open(QIODevice::WriteOnly)) {
            QMutexLocker locker(&m_Mutex);
            Q_UNUSED(locker);

            QTextStream stream(&userDictonaryFile);

            for (const QString &word: m_WordsList) {
                stream << word << endl;
            }
        } else {
            LOG_WARNING << "Unable to save user dictionary";
        }
    }

    QStringList UserDictionary::getWords() {
        QMutexLocker locker(&m_Mutex);
        Q_UNUSED(locker);
        return m_WordsList;
    }

    int UserDictionary::getWordsCount() {
        QMutexLocker locker(&m_Mutex);
        Q_UNUSED(locker);
        return m_WordsList.size();
    }

    bool UserDictionary::contains(const QString &word) {
        QMutexLocker locker(&m_Mutex);
        Q_UNUSED(locker);
        return m_WordsSet.contains(word.toLower());
    }

    void UserDictionary::addWords(const QStringList &words) {
        {
            QMutexLocker locker(&m_Mutex);
            Q_UNUSED(locker);

            addWordsUnsafe(words);
        }
        emit sizeChanged();
    }

    void UserDictionary::reset(const QStringList &words) {
        LOG_DEBUG << "#";
        {
            QMutexLocker locker(&m_Mutex);
            Q_UNUSED(locker);

            clearUnsafe();
            addWordsUnsafe(words);
        }
        emit sizeChanged();
        emit userDictUpdate(words, true);
    }

    void UserDictionary::addWord(const QString &word) {
        LOG_INFO << word;
        {
            QMutexLocker locker(&m_Mutex);
            Q_UNUSED(locker);

            QStringList parts;
            Helpers::splitText(word, parts);

            for (auto &part: parts) {
                addWordUnsafe(part);
            }
        }
        emit sizeChanged();
        emit userDictUpdate(QStringList() << word, false);
    }

    void UserDictionary::clear() {
        {
            QMutexLocker locker(&m_Mutex);
            Q_UNUSED(locker);
            clearUnsafe();
        }
        emit sizeChanged();
        emit userDictCleared();
    }

    bool UserDictionary::empty() {
        QMutexLocker locker(&m_Mutex);
        Q_UNUSED(locker);
        return m_WordsSet.isEmpty();
    }

    int UserDictionary::size()  {
        QMutexLocker locker(&m_Mutex);
        Q_UNUSED(locker);
        return m_WordsList.size();
    }

    void UserDictionary::addWordsUnsafe(const QStringList &words) {
        QStringList parts;

        foreach (const QString &word, words) {
            Helpers::splitText(word, parts);
        }

        for (auto &part: parts) {
            addWordUnsafe(part);
        }
    }

    void UserDictionary::addWordUnsafe(const QString &word) {
        QString wordToAdd = word.toLower();
        if (!m_WordsSet.contains(wordToAdd)) {
            m_WordsSet.insert(wordToAdd);
            m_WordsList.append(word);
        }
    }
}

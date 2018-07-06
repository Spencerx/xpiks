/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef USERDICTIONARY_H
#define USERDICTIONARY_H

#include <QObject>
#include <QMutex>
#include <QString>
#include <QSet>
#include <QStringList>
#include <Common/isystemenvironment.h>

namespace SpellCheck {
    class UserDictionary: public QObject {
        Q_OBJECT
    public:
        UserDictionary(Common::ISystemEnvironment &environment);

    public:
        void initialize();
        void save();

    public:
        QStringList getWords();
        bool contains(const QString &word);

        void addWords(const QStringList &words);
        void reset(const QStringList &words);
        void addWord(const QString &word);

        void clear();
        bool empty();
        int size();

    signals:
        void sizeChanged();
        void userDictCleared();
        void userDictUpdate(const QStringList &words, bool override);

    private:
        void addWordsUnsafe(const QStringList &words);
        void addWordUnsafe(const QString &word);
        void clearUnsafe() { m_WordsList.clear(); m_WordsSet.clear(); }

    private:
        QMutex m_Mutex;
        QString m_Filepath;
        QSet<QString> m_WordsSet;
        QStringList m_WordsList;
    };
}

#endif // USERDICTIONARY_H

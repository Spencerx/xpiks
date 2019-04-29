/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef TRANSLATIONQUERY_H
#define TRANSLATIONQUERY_H

#include <QObject>
#include <QString>

namespace Translation {
    class TranslationQuery : public QObject
    {
        Q_OBJECT
    public:
        explicit TranslationQuery(const QString &query, QObject *parent = nullptr);

    public:
        const QString &getQuery() const { return m_Query; }

    public:
        void notifyTranslated() { emit translationAvailable(); }

    signals:
        void translationAvailable();

    private:
        QString m_Query;
    };
}

#endif // TRANSLATIONQUERY_H

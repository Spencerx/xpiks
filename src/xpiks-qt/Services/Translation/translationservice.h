/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef TRANSLATIONSERVICE_H
#define TRANSLATIONSERVICE_H

#include <QObject>
#include <QString>

namespace Helpers {
    class AsyncCoordinator;
}

namespace Translation {
    class TranslationWorker;

    class TranslationService :
            public QObject
    {
        Q_OBJECT
    public:
        explicit TranslationService(QObject *parent = nullptr);

        void startService(Helpers::AsyncCoordinator &coordinator);
        void stopService();

        bool isAvailable() const { return true; }
        bool isBusy() const;

    public:
        void selectDictionary(const QString &dictionaryPath);
        void translate(const QString &what);
        bool retrieveTranslation(const QString &what, QString &translation);

    signals:
        void translationAvailable();

    private slots:
        void workerFinished();
        void workerDestroyed(QObject* object);

    private:
        TranslationWorker *m_TranslationWorker;
    };
}

#endif // TRANSLATIONSERVICE_H

/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2017 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef LIBRARYLOADERWORKER_H
#define LIBRARYLOADERWORKER_H

#include <QObject>

namespace Suggestion {
    class LocalLibrary;

    class LibraryLoaderWorker : public QObject
    {
        Q_OBJECT
    public:
        enum LoadOption {
            Load, Save, Clean
        };

    public:
        LibraryLoaderWorker(Suggestion::LocalLibrary *localLibrary, const QString &filepath, LoadOption option);

    signals:
        void stopped();

    public slots:
        void process();

    private:
        void read();
        void write();
        void cleanup();

    private:
        Suggestion::LocalLibrary *m_LocalLibrary;
        QString m_Filepath;
        LoadOption m_Option;
    };
}

#endif // LIBRARYLOADERWORKER_H

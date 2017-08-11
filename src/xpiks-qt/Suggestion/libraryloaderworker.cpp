/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2017 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QFile>
#include <QHash>
#include <QString>
#include <QStringList>
#include <QDataStream>
#include "libraryloaderworker.h"
#include "locallibrary.h"
#include "../Common/defines.h"

namespace Suggestion {
    LibraryLoaderWorker::LibraryLoaderWorker(Suggestion::LocalLibrary *localLibrary, const QString &filepath, LoadOption option):
        m_LocalLibrary(localLibrary),
        m_Filepath(filepath),
        m_Option(option)
    {
    }

    void LibraryLoaderWorker::process() {
        LOG_DEBUG << "#";

        if (m_Option == Save) {
            write();
        } else if (m_Option == Load) {
            read();
        } else if (m_Option == Clean) {
            cleanup();
        }

        emit stopped();
    }

    void LibraryLoaderWorker::read() {
        LOG_DEBUG << "#";

        QFile file(m_Filepath);
        if (file.open(QIODevice::ReadOnly)) {
            QHash<QString, LocalArtworkData> dict;

            QDataStream in(&file);   // read the data
            in >> dict;
            file.close();

            m_LocalLibrary->swap(dict);
        } else {
            LOG_WARNING << "Failed to open" << m_Filepath;
        }
    }

    void LibraryLoaderWorker::write() {
        LOG_DEBUG << "#";

        m_LocalLibrary->saveToFile();
    }

    void LibraryLoaderWorker::cleanup() {
        LOG_DEBUG << "#";
        m_LocalLibrary->cleanupTrash();
    }
}


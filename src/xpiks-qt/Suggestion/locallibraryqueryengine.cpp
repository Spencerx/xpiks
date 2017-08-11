/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2017 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "locallibraryqueryengine.h"

#include <QThread>
#include "libraryqueryworker.h"
#include "../Common/defines.h"
#define MAX_LOCAL_RESULTS 200

namespace Suggestion {
    LocalLibraryQueryEngine::LocalLibraryQueryEngine(int engineID, LocalLibrary *localLibrary):
        SuggestionQueryEngineBase(engineID),
        m_LocalLibrary(localLibrary)
    {
    }

    void LocalLibraryQueryEngine::submitQuery(const QStringList &queryKeywords, QueryResultsType resultsType) {
        LOG_DEBUG << queryKeywords;
        Q_UNUSED(resultsType);
        LibraryQueryWorker *worker = new LibraryQueryWorker(m_LocalLibrary, queryKeywords, MAX_LOCAL_RESULTS);
        QThread *thread = new QThread();
        worker->moveToThread(thread);

        QObject::connect(thread, SIGNAL(started()), worker, SLOT(process()));
        QObject::connect(worker, SIGNAL(stopped()), thread, SLOT(quit()));

        QObject::connect(worker, SIGNAL(stopped()), worker, SLOT(deleteLater()));
        QObject::connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));

        QObject::connect(this, SIGNAL(cancelAllQueries()),
                         worker, SLOT(cancel()));

        QObject::connect(worker, SIGNAL(resultsFound()),
                         this, SLOT(resultsFoundHandler()));

        thread->start();
    }

    void LocalLibraryQueryEngine::resultsFoundHandler() {
        LibraryQueryWorker *worker = qobject_cast<LibraryQueryWorker*>(sender());
        setResults(worker->getResults());
        worker->doShutdown();
        emit resultsAvailable();
    }
}

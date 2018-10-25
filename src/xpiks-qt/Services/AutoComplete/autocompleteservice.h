/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef AUTOCOMPLETESERVICE_H
#define AUTOCOMPLETESERVICE_H

#include <memory>

#include <QObject>
#include <QString>

class QThread;

namespace Artworks {
    class BasicKeywordsModel;
}

namespace KeywordsPresets {
    class PresetKeywordsModel;
}

namespace Models {
    class SettingsModel;
}

namespace Helpers {
    class AsyncCoordinator;
}

namespace AutoComplete {
    class AutoCompleteWorker;
    class KeywordsAutoCompleteModel;
    class CompletionQuery;

    class AutoCompleteService:
            public QObject
    {
        Q_OBJECT
    public:
        AutoCompleteService(Models::SettingsModel &settingsModel,
                            KeywordsAutoCompleteModel &autoCompleteModel,
                            QObject *parent = 0);

    public:
        void startService(Helpers::AsyncCoordinator &coordinator,
                          KeywordsPresets::PresetKeywordsModel &presetsManager);
        void stopService();

        bool isAvailable() const { return true; }
        bool isBusy() const;

    public:
        void generateCompletions(QString const &prefix, Artworks::BasicKeywordsModel &basicModel);
        void generateCompletions(QString const &prefix);

    private:
        std::shared_ptr<CompletionQuery> createQuery(QString const &prefix);

    private slots:
        void workerFinished();
        void workerDestroyed(QObject* object);

    signals:
        void cancelAutoCompletion();
        void serviceAvailable();

    private:
        QThread *m_WorkerThread;
        AutoCompleteWorker *m_AutoCompleteWorker;
        KeywordsAutoCompleteModel &m_AutoCompleteModel;
        Models::SettingsModel &m_SettingsModel;
    };
}

#endif // AUTOCOMPLETESERVICE_H

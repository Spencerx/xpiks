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

#include <QObject>
#include <QString>
#include <memory>

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

    class AutoCompleteService:
            public QObject
    {
        Q_OBJECT
    public:
        AutoCompleteService(KeywordsAutoCompleteModel &autoCompleteModel,
                            KeywordsPresets::PresetKeywordsModel &presetsManager,
                            Models::SettingsModel &settingsModel,
                            QObject *parent = 0);

    public:
        void startService(Helpers::AsyncCoordinator &coordinator);
        void stopService();

        bool isAvailable() const { return true; }
        bool isBusy() const;

    public:
        void generateCompletions(const QString &prefix, Artworks::BasicKeywordsModel *basicModel);

    private slots:
        void workerFinished();
        void workerDestroyed(QObject* object);

    signals:
        void cancelAutoCompletion();
        void serviceAvailable();

#ifdef INTEGRATION_TESTS
    public:
        KeywordsAutoCompleteModel *getAutoCompleteModel() const { return m_AutoCompleteModel; }
#endif

    private:
        AutoCompleteWorker *m_AutoCompleteWorker;
        KeywordsAutoCompleteModel &m_AutoCompleteModel;
        KeywordsPresets::PresetKeywordsModel &m_PresetsManager;
        Models::SettingsModel &m_SettingsModel;
    };
}

#endif // AUTOCOMPLETESERVICE_H

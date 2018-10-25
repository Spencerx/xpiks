/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef WARNINGSSERVICE_H
#define WARNINGSSERVICE_H

#include <memory>

#include <QObject>
#include <QString>

#include "Common/flags.h"

namespace Artworks {
    class ArtworkMetadata;
    class ArtworksSnapshot;
}

namespace Warnings {
    class WarningsCheckingWorker;
    class WarningsSettingsModel;

    class WarningsService: public QObject
    {
        Q_OBJECT

    public:
        explicit WarningsService(WarningsSettingsModel &warningsSettingsModel, QObject *parent=0);

    public:
        void initWarningsSettings();
        void cancelCurrentBatch();

    public:
        virtual void startService();
        virtual void stopService();

        virtual bool isAvailable() const { return true; }
        virtual bool isBusy() const;

        virtual void submitItem(std::shared_ptr<Artworks::ArtworkMetadata> const &item);
        virtual void submitItem(std::shared_ptr<Artworks::ArtworkMetadata> const &item,
                                Common::WarningsCheckFlags flags);
        virtual void submitItems(const Artworks::ArtworksSnapshot &items);

    private slots:
        void workerDestoyed(QObject *object);
        void workerStopped();

    signals:
        void queueIsEmpty();

    private:
        WarningsCheckingWorker *m_WarningsWorker;
        WarningsSettingsModel &m_WarningsSettingsModel;
        bool m_IsStopped;
    };
}

#endif // WARNINGSSERVICE_H

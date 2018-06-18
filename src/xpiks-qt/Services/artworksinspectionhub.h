/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef ARTWORKSINSPECTIONHUB_H
#define ARTWORKSINSPECTIONHUB_H

#include <QObject>
#include "../Artworks/artworkssnapshot.h"

namespace SpellCheck {
    class SpellCheckerService;
}

namespace Warnings {
    class WarningsService;
}

namespace Services {
    class ArtworksInspectionHub : public QObject
    {
        Q_OBJECT
    public:
        explicit ArtworksInspectionHub(SpellCheck::SpellCheckerService &spellcheckService,
                                       Warnings::WarningsService &warningsService,
                                       QObject *parent = 0);

    public:
        void checkArtworks(const Artworks::WeakArtworksSnapshot &artworks) const;
        void checkArtworks(const Artworks::ArtworksSnapshot::Container &artworks) const;

    signals:
        void updateRequested();

    private slots:
        void onUpdateRequested();
        void onUpdateTimer();

    private:
        QMutex m_Lock;
        SpellCheck::SpellCheckerService &m_SpellCheckService;
        Warnings::WarningsService &m_WarningsService;
        QTimer m_UpdateTimer;
        int m_TimerRestartedCount;
    };
}

#endif // ARTWORKSINSPECTIONHUB_H

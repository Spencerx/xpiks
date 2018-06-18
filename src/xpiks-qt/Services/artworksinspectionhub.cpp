/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "artworksinspectionhub.h"
#include "../SpellCheck/spellcheckerservice.h"
#include "../Warnings/warningsservice.h"

#define MAX_UPDATE_TIMER_DELAYS 2
#define UPDATE_TIMER_DELAY 200

namespace Services {
    ArtworksInspectionHub::ArtworksInspectionHub(SpellCheck::SpellCheckerService &spellcheckService,
                                                 Warnings::WarningsService &warningsService,
                                                 QObject *parent):
        QObject(parent),
        m_SpellCheckService(spellcheckService),
        m_WarningsService(warningsService),
        m_TimerRestartedCount(0)
    {
        m_UpdateTimer.setSingleShot(true);

        QObject::connect(&m_UpdateTimer, SIGNAL(timeout()), this, SLOT(onUpdateTimer()));
        QObject::connect(this, SIGNAL(updateRequested()), this, SLOT(onUpdateRequested()));
    }

    void ArtworksInspectionHub::checkArtworks(const Artworks::WeakArtworksSnapshot &artworks) const {
        m_Artworks.append(artworks);
        emit updateRequested();
    }

    void ArtworksInspectionHub::checkArtworks(const Artworks::ArtworksSnapshot::Container &artworks) const {
        m_Artworks.append(artworks);
        emit updateRequested();
    }

    void ArtworksInspectionHub::onUpdateRequested() {
        LOG_DEBUG << "#";
        /*
         * Force update might be dangerous because it is possible
         * that restart count will be high but timer is not started
         */

        if (m_TimerRestartedCount < MAX_UPDATE_TIMER_DELAYS) {
            m_UpdateTimer.start(UPDATE_TIMER_DELAY);

            QMutexLocker locker(&m_Lock);
            Q_UNUSED(locker);
            m_TimerRestartedCount++;
        } else {
            LOG_INFO << "Maximum backup delays occured, forcing update";
            Q_ASSERT(m_UpdateTimer.isActive());
        }
    }

    void ArtworksInspectionHub::onUpdateTimer() {
    }
}

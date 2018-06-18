/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef ARTWORKSUPDATEHUB_H
#define ARTWORKSUPDATEHUB_H

#include <QObject>
#include <QMutex>
#include <QTimer>
#include <QSet>
#include <vector>
#include <memory>
#include "artworksupdatehub.h"
#include "../Artworks/artworkssnapshot.h"

namespace Artworks {
    class ArtworkMetadata;
}

namespace Models {
    class ArtworksListModel;
}

namespace QMLExtensions {
    class ArtworkUpdateRequest;

    class ArtworksUpdateHub : public QObject
    {
        Q_OBJECT
    public:
        explicit ArtworksUpdateHub(Models::ArtworksListModel &artworksListModel, QObject *parent = 0);

    public:
        void setStandardRoles(const QVector<int> &roles);

    public:
        void updateArtwork(qint64 artworkID, size_t lastKnownIndex, const QSet<int> &rolesToUpdate = QSet<int>());
        void updateArtwork(Artworks::ArtworkMetadata *artwork);
        void updateArtworks(const Artworks::WeakArtworksSnapshot &artworks, bool fastUpdate=false) const;

#ifdef INTEGRATION_TESTS
    public:
        void clear();
#endif

    signals:
        void updateRequested();

    private slots:
        void onUpdateRequested();
        void onUpdateTimer();

    private:
        QMutex m_Lock;
        QSet<int> m_StandardRoles;
        Models::ArtworksListModel &m_ArtworksListModel;
        std::vector<std::shared_ptr<ArtworkUpdateRequest> > m_UpdateRequests;
        QTimer m_UpdateTimer;
        int m_TimerRestartedCount;
    };
}

#endif // ARTWORKSUPDATEHUB_H

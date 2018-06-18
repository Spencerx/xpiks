/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "artworksupdatehub.h"
#include "artworkupdaterequest.h"
#include "../Commands/commandmanager.h"
#include "../Models/artworkslistmodel.h"
#include "../Models/artworkproxymodel.h"
#include "../Common/defines.h"

#define MAX_NOT_UPDATED_ARTWORKS_TO_HOLD 50
#define MAX_UPDATE_TIMER_DELAYS 2
#define UPDATE_TIMER_DELAY 400

namespace Services {
    ArtworksUpdateHub::ArtworksUpdateHub(Models::ArtworksListModel &artworksListModel, QObject *parent) :
        QObject(parent),
        m_ArtworksListModel(artworksListModel),
        m_TimerRestartedCount(0)
    {
        m_UpdateTimer.setSingleShot(true);

        QObject::connect(&m_UpdateTimer, SIGNAL(timeout()), this, SLOT(onUpdateTimer()));
        QObject::connect(this, SIGNAL(updateRequested()), this, SLOT(onUpdateRequested()));

        m_UpdateRequests.reserve(100);
    }

    void ArtworksUpdateHub::setStandardRoles(const QVector<int> &roles) {
        m_StandardRoles = roles.toList().toSet();
    }

    void ArtworksUpdateHub::updateArtwork(qint64 artworkID, size_t lastKnownIndex, const QSet<int> &rolesToUpdate) {
        std::shared_ptr<ArtworkUpdateRequest> updateRequest(
                    new ArtworkUpdateRequest(artworkID, lastKnownIndex, rolesToUpdate));
        {
            QMutexLocker locker(&m_Lock);
            m_UpdateRequests.emplace_back(updateRequest);
        }

        emit updateRequested();
    }

    void ArtworksUpdateHub::updateArtwork(Artworks::ArtworkMetadata *artwork) {
        Q_ASSERT(artwork != nullptr);
        this->updateArtwork(artwork->getItemID(), artwork->getLastKnownIndex(), m_StandardRoles);
    }

    void ArtworksUpdateHub::updateArtworks(const Artworks::WeakArtworksSnapshot &artworks, UpdateMode updateMode) const {
        decltype(m_UpdateRequests) requests;
        requests.reserve(artworks.size());
        for (auto &artwork: artworks) {
            std::shared_ptr<ArtworkUpdateRequest> updateRequest(
                        new ArtworkUpdateRequest(
                            artwork->getItemID(),
                            artwork->getLastKnownIndex(),
                            m_StandardRoles,
                            updateMode == FastUpdate));
            requests.emplace_back(updateRequest);
        }

        {
            QMutexLocker locker(&m_Lock);
            m_UpdateRequests.insert(m_UpdateRequests.end(), requests.begin(), requests.end());
        }

        emit updateRequested();
    }

    void ArtworksUpdateHub::updateArtworks(const Artworks::ArtworksSnapshot::Container &artworks, UpdateMode updateMode) const {
        decltype(m_UpdateRequests) requests;
        requests.reserve(artworks.size());
        for (auto &locker: artworks) {
            auto *artwork = locker->getArtworkMetadata();
            std::shared_ptr<ArtworkUpdateRequest> updateRequest(
                        new ArtworkUpdateRequest(
                            artwork->getItemID(),
                            artwork->getLastKnownIndex(),
                            m_StandardRoles,
                            updateMode == FastUpdate));
            requests.emplace_back(updateRequest);
        }

        {
            QMutexLocker locker(&m_Lock);
            m_UpdateRequests.insert(m_UpdateRequests.end(), requests.begin(), requests.end());
        }

        emit updateRequested();
    }

#ifdef INTEGRATION_TESTS
        void ArtworksUpdateHub::clear() {
            {
                QMutexLocker locker(&m_Lock);
                m_UpdateRequests.clear();
            }
        }
#endif

    void ArtworksUpdateHub::onUpdateRequested() {
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

    void ArtworksUpdateHub::onUpdateTimer() {
        LOG_DEBUG << "#";
        std::vector<std::shared_ptr<ArtworkUpdateRequest> > requests;

        {
            QMutexLocker locker(&m_Lock);
            Q_UNUSED(locker);
            requests.swap(m_UpdateRequests);
            m_TimerRestartedCount = 0;
        }

        m_ArtworksListModel.processUpdateRequests(requests);

        QSet<qint64> artworkIDsToUpdate;
        QSet<int> commonRoles;

        const size_t size = requests.size();
        std::vector<std::shared_ptr<ArtworkUpdateRequest> > requestsToResubmit;
        requestsToResubmit.reserve(size / 3);

        for (auto &request: requests) {
            if (!request->isCacheMiss()) { continue; }
            if (request->isFastUpdate()) { continue; }

            // allow cache miss requests to be postponed 1 time
            // in order to gather more of them and process in 1 go later
            if (request->isFirstGeneration()) {
                request->incrementGeneration();
                requestsToResubmit.emplace_back(request);
            } else {
                artworkIDsToUpdate.insert(request->getArtworkID());
                commonRoles.unite(request->getRolesToUpdate());
            }
        }

        LOG_INFO << requestsToResubmit.size() << "requests to resubmit";
        if (!requestsToResubmit.empty()) {
            {
                QMutexLocker locker(&m_Lock);
                m_UpdateRequests.insert(m_UpdateRequests.end(), requestsToResubmit.begin(), requestsToResubmit.end());
            }

            emit updateRequested();
        }

        QVector<int> roles = commonRoles.toList().toVector();
        m_ArtworksListModel.updateArtworksByIDs(artworkIDsToUpdate, roles);
    }
}

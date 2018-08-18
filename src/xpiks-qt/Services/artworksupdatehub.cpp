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
#include <Common/defines.h>
#include <Models/Artworks/artworkslistmodel.h>
#include <Artworks/artworkssnapshot.h>

#define MAX_NOT_UPDATED_ARTWORKS_TO_HOLD 50
#ifndef CORE_TESTS
#define MAX_UPDATE_TIMER_DELAYS 2
#define UPDATE_TIMER_DELAY 400
#else
#define MAX_UPDATE_TIMER_DELAYS 1
#define UPDATE_TIMER_DELAY 0
#endif

namespace Services {
    ArtworksUpdateHub::ArtworksUpdateHub(Models::ArtworksListModel &artworksListModel,
                                         QObject *parent) :
        QObject(parent),
        m_ArtworksListModel(artworksListModel),
        m_TimerRestartedCount(0)
    {
        m_UpdateTimer.setSingleShot(true);

        QObject::connect(&m_UpdateTimer, SIGNAL(timeout()), this, SLOT(onUpdateTimer()));
        QObject::connect(this, SIGNAL(updateRequested()), this, SLOT(onUpdateRequested()));

        m_UpdateRequests.reserve(100);

        m_StandardRoles = artworksListModel.getStandardUpdateRoles();
    }

    void ArtworksUpdateHub::updateArtworkByID(Common::ID_t artworkID,
                                          size_t lastKnownIndex,
                                          const QVector<int> &rolesToUpdate) {
        auto updateRequest = std::make_shared<ArtworkUpdateRequest>(
                                 artworkID, lastKnownIndex, rolesToUpdate.toList().toSet());
        {
            QMutexLocker locker(&m_Lock);
            m_UpdateRequests.emplace_back(updateRequest);
        }

        emit updateRequested();
    }

    void ArtworksUpdateHub::updateArtwork(std::shared_ptr<Artworks::ArtworkMetadata> const &artwork) {
        Q_ASSERT(artwork != nullptr);
        this->updateArtworkByID(artwork->getItemID(), artwork->getLastKnownIndex(), m_StandardRoles);
    }

    void ArtworksUpdateHub::updateArtworks(Artworks::WeakArtworksSnapshot const &artworks, UpdateMode updateMode) {
        LOG_INFO << "Update" << artworks.size() << "artwork(s)";
        decltype(m_UpdateRequests) requests;
        requests.reserve(artworks.size());
        QSet<int> rolesToUpdate = m_StandardRoles.toList().toSet();
        for (auto &artwork: artworks) {
            auto updateRequest = std::make_shared<ArtworkUpdateRequest>(
                            artwork->getItemID(),
                            artwork->getLastKnownIndex(),
                            rolesToUpdate,
                            updateMode == FastUpdate);
            requests.emplace_back(updateRequest);
        }

        {
            QMutexLocker locker(&m_Lock);
            m_UpdateRequests.insert(m_UpdateRequests.end(), requests.begin(), requests.end());
        }

        emit updateRequested();
    }

    void ArtworksUpdateHub::updateArtworks(Artworks::ArtworksSnapshot const &artworks, UpdateMode updateMode) {
        LOG_INFO << "Update" << artworks.size() << "artwork(s)";
        if (artworks.empty()) { return; }
        decltype(m_UpdateRequests) requests;
        requests.reserve(artworks.size());
        QSet<int> rolesToUpdate = m_StandardRoles.toList().toSet();
        for (auto &locker: artworks.getRawData()) {
            auto *artwork = locker->getArtworkMetadata();
            auto updateRequest = std::make_shared<ArtworkUpdateRequest>(
                            artwork->getItemID(),
                            artwork->getLastKnownIndex(),
                            rolesToUpdate,
                            updateMode == FastUpdate);
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

        resubmitFailedRequests(requests);
    }

    void ArtworksUpdateHub::resubmitFailedRequests(std::vector<std::shared_ptr<ArtworkUpdateRequest> > &requests) {
        QSet<qint64> artworkIDsToUpdate;
        QSet<int> commonRoles;

        const size_t size = requests.size();
        std::vector<std::shared_ptr<ArtworkUpdateRequest>> requestsToResubmit;
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
                artworkIDsToUpdate.insert(request->getArtworkID().get());
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

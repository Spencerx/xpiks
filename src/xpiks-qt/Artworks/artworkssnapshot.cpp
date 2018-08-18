/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "artworkssnapshot.h"

namespace Artworks {
    ArtworkSessionSnapshot::ArtworkSessionSnapshot(ArtworkMetadata *metadata) {
        Q_ASSERT(metadata != nullptr);
        m_ArtworkPath = metadata->getFilepath();

        ImageArtwork *image = dynamic_cast<ImageArtwork*>(metadata);
        if (image != nullptr && image->hasVectorAttached()){
            m_VectorPath = image->getAttachedVectorPath();
        }
    }

    SessionSnapshot::SessionSnapshot(std::vector<ArtworkMetadata *> const &artworksList, QStringList const &fullDirectories):
        m_DirectoriesSnapshot(fullDirectories)
    {
        LOG_DEBUG << "Creating snapshot of" << artworksList.size() << "artwork(s) and" << m_DirectoriesSnapshot.size() << "full directory(ies)";

        m_ArtworksSnapshot.reserve(artworksList.size());
        for (const auto &artwork: artworksList) {
            m_ArtworksSnapshot.emplace_back(std::make_shared<ArtworkSessionSnapshot>(artwork));
        }
    }

    ArtworksSnapshot::ArtworksSnapshot(std::initializer_list<ItemType> artworks):
        m_ArtworksSnapshot(artworks)
    {
    }

    ArtworksSnapshot::ArtworksSnapshot(std::deque<ItemType> const &artworks):
        m_ArtworksSnapshot(artworks.begin(), artworks.end())
    {
    }

    ArtworksSnapshot::ArtworksSnapshot(Container &rawSnapshot) {
        m_ArtworksSnapshot = std::move(rawSnapshot);
    }

    ArtworksSnapshot::ArtworksSnapshot(ArtworksSnapshot &&other):
        m_ArtworksSnapshot(std::move(other.m_ArtworksSnapshot))
    {
    }

    ArtworksSnapshot &ArtworksSnapshot::operator=(ArtworksSnapshot &&other) {
        if (this != &other) {
            LOG_DEBUG << "Moving snapshot of" << other.m_ArtworksSnapshot.size() << "item(s)";
            m_ArtworksSnapshot = std::move(other.m_ArtworksSnapshot);
        }

        return *this;
    }

    ArtworksSnapshot::~ArtworksSnapshot() {
        LOG_DEBUG << "Destroying snapshot of" << m_ArtworksSnapshot.size() << "artwork(s)";
    }

    void ArtworksSnapshot::append(ArtworksSnapshot::ItemType const &item) {
        m_ArtworksSnapshot.push_back(item);
    }

    void ArtworksSnapshot::append(std::deque<ItemType> const &artworks) {
        LOG_DEBUG << "Appending snapshot of" << artworks.size() << "artwork(s)";
        m_ArtworksSnapshot.insert(m_ArtworksSnapshot.end(), artworks.begin(), artworks.end());
    }

    void ArtworksSnapshot::append(Container const &rawSnapshot) {
        LOG_DEBUG << "Appending snapshot of" << rawSnapshot.size() << "artwork(s)";

        m_ArtworksSnapshot.insert(m_ArtworksSnapshot.end(), rawSnapshot.begin(), rawSnapshot.end());
    }

    void ArtworksSnapshot::set(Container &rawSnapshot) {
        m_ArtworksSnapshot = std::move(rawSnapshot);
    }

    void ArtworksSnapshot::copyFrom(const ArtworksSnapshot &snapshot) {
        LOG_DEBUG << "Copying" << snapshot.size() << "artwork(s)";
        m_ArtworksSnapshot.clear();
        m_ArtworksSnapshot = snapshot.m_ArtworksSnapshot;
    }

    void ArtworksSnapshot::remove(size_t index) {
        if (index >= m_ArtworksSnapshot.size()) { return; }

        m_ArtworksSnapshot.erase(m_ArtworksSnapshot.begin() + index);
    }

    void ArtworksSnapshot::clear() {
        LOG_DEBUG << "Removing" << size() << "item(s)";
        m_ArtworksSnapshot.clear();
    }
}

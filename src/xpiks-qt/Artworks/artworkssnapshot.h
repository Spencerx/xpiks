/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef ARTWORKMETADATASNAPSHOT_H
#define ARTWORKMETADATASNAPSHOT_H

#include <QString>
#include <deque>
#include <memory>
#include <initializer_list>
#include "artworkmetadata.h"
#include "imageartwork.h"

namespace Artworks {
    class ArtworkSessionSnapshot
    {
    public:
        ArtworkSessionSnapshot(ArtworkMetadata *metadata);

    public:
        const QString &getArtworkFilePath() const { return m_ArtworkPath; }
        const QString &getAttachedVectorPath() const { return m_VectorPath; }

    private:
        QString m_ArtworkPath;
        QString m_VectorPath;
    };

    class SessionSnapshot {
    public:
        SessionSnapshot(std::vector<ArtworkMetadata *> const &artworksList, QStringList const &fullDirectories);

        SessionSnapshot(SessionSnapshot &&other) {
            m_ArtworksSnapshot.swap(other.m_ArtworksSnapshot);
            m_DirectoriesSnapshot.swap(other.m_DirectoriesSnapshot);
        }

        SessionSnapshot &operator=(SessionSnapshot &&other) {
            if (this != &other) {
                m_ArtworksSnapshot.swap(other.m_ArtworksSnapshot);
                m_DirectoriesSnapshot.swap(other.m_DirectoriesSnapshot);
            }

            return *this;
        }

    public:
        std::vector<std::shared_ptr<ArtworkSessionSnapshot> > &getSnapshot() { return m_ArtworksSnapshot; }
        const QStringList &getDirectoriesSnapshot() { return m_DirectoriesSnapshot; }

    private:
        std::vector<std::shared_ptr<ArtworkSessionSnapshot> > m_ArtworksSnapshot;
        QStringList m_DirectoriesSnapshot;
    };

    class ArtworksSnapshot {
    public:
        using ItemType = std::shared_ptr<Artworks::ArtworkMetadata>;
        using Container = std::vector<ItemType>;

    public:
        ArtworksSnapshot() { }
        ArtworksSnapshot(std::initializer_list<ItemType> artworks);
        ArtworksSnapshot(const std::deque<ItemType> &artworks);
        ArtworksSnapshot(Container &rawSnapshot);
        ArtworksSnapshot(ArtworksSnapshot &&other);
        ArtworksSnapshot &operator=(ArtworksSnapshot &&other);
        ArtworksSnapshot(const ArtworksSnapshot &) = delete;
        ArtworksSnapshot &operator=(ArtworksSnapshot &) = delete;
        virtual ~ArtworksSnapshot();

    public:
        const Container &getRawData() const { return m_ArtworksSnapshot; }
        size_t size() const { return m_ArtworksSnapshot.size(); }

    public:
        void reserve(size_t size) { m_ArtworksSnapshot.reserve(size); }
        void append(ItemType const &artwork);
        void append(const std::deque<ItemType> &artworks);
        void append(Container const &rawSnapshot);
        void set(Container &rawSnapshot);
        void remove(size_t index);
        ItemType const &get(size_t i) const { Q_ASSERT(i < m_ArtworksSnapshot.size()); return m_ArtworksSnapshot.at(i); }
        ItemType const &at(size_t i) const { return get(i); }
        void clear();
        bool empty() const { return m_ArtworksSnapshot.empty(); }

    private:
        Container m_ArtworksSnapshot;
    };
}

#endif // ARTWORKMETADATASNAPSHOT_H

/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "artworksviewmodel.h"
#include <QVector>
#include <Helpers/indiceshelper.h>
#include <Common/defines.h>
#include <Artworks/imageartwork.h>
#include <Artworks/videoartwork.h>
#include <Helpers/cpphelpers.h>

namespace Models {
    ArtworksViewModel::ArtworksViewModel(QObject *parent):
        AbstractListModel(parent)
    {
    }

    void ArtworksViewModel::setArtworks(const Artworks::ArtworksSnapshot &snapshot) {
        LOG_INFO << snapshot.size() << "artworks";
        if (snapshot.empty()) { return; }

        using namespace Artworks;
        auto elements = Helpers::map<std::shared_ptr<ArtworkMetadataLocker>, std::shared_ptr<ArtworkMetadataLocker>>(
                            snapshot.getRawData(),
                            [](const std::shared_ptr<ArtworkMetadataLocker> &locker) {
            return std::make_shared<ArtworkElement>(locker->getArtworkMetadata());
        });

        beginResetModel();
        {
            m_ArtworksSnapshot.set(elements);
        }
        endResetModel();

        emit artworksCountChanged();
    }

    int ArtworksViewModel::getSelectedArtworksCount() const {
        int selectedCount = 0;
        size_t size = m_ArtworksSnapshot.size();
        for (size_t i = 0; i < size; i++) {
            if (getIsSelected(i)) {
                selectedCount++;
            }
        }

        return selectedCount;
    }

    void ArtworksViewModel::setArtworkSelected(int index, bool value) {
        if (index < 0 || index >= (int)m_ArtworksSnapshot.size()) {
            return;
        }

        setIsSelected(index, value);

        QModelIndex qIndex = this->index(index);
        emit dataChanged(qIndex, qIndex, QVector<int>() << IsSelectedRole);
        emit selectedArtworksCountChanged();
    }

    void ArtworksViewModel::unselectAllItems() {
        const size_t size = m_ArtworksSnapshot.size();
        for (size_t i = 0; i < size; i++) {
            setIsSelected(i, false);
        }

        emit dataChanged(this->index(0), this->index(rowCount() - 1), QVector<int>() << IsSelectedRole);
    }

    Artworks::ArtworkElement *ArtworksViewModel::accessItem(size_t index) const {
        Q_ASSERT(index < m_ArtworksSnapshot.size());
        auto &locker = m_ArtworksSnapshot.at(index);
        auto element = std::dynamic_pointer_cast<Artworks::ArtworkElement>(locker);
        Q_ASSERT(element);
        return element.get();
    }

    bool ArtworksViewModel::getIsSelected(size_t i) const {
        auto &locker = m_ArtworksSnapshot.at(i);
        auto element = std::dynamic_pointer_cast<Artworks::ArtworkElement>(locker);
        Q_ASSERT(element);
        bool result = element->getIsSelected();
        return result;
    }

    void ArtworksViewModel::setIsSelected(size_t i, bool value) {
        auto &locker = m_ArtworksSnapshot.at(i);
        auto element = std::dynamic_pointer_cast<Artworks::ArtworkElement>(locker);
        Q_ASSERT(element);
        element->setIsSelected(value);
    }

    Artworks::ArtworkMetadata *ArtworksViewModel::getArtworkMetadata(size_t i) const {
        Q_ASSERT((i >= 0) && (i < m_ArtworksSnapshot.size()));
        return m_ArtworksSnapshot.get(i);
    }

    /*virtual*/
    bool ArtworksViewModel::doRemoveSelectedArtworks() {
        LOG_DEBUG << "#";

        const size_t size = m_ArtworksSnapshot.size();
        std::vector<int> indicesToRemove;
        indicesToRemove.reserve(size);

        for (size_t i = 0; i < size; ++i) {
            if (getIsSelected(i)) {
                indicesToRemove.push_back((int)i);
            }
        }

        const bool anyItemToRemove = !indicesToRemove.empty();
        if (anyItemToRemove) {
            LOG_INFO << "Removing" << indicesToRemove.size() << "item(s)";
            removeItems(Helpers::IndicesRanges(indicesToRemove));

            if (m_ArtworksSnapshot.empty()) {
                emit requestCloseWindow();
            }

            emit artworksCountChanged();
        }

        return anyItemToRemove;
    }

    void ArtworksViewModel::doResetModel() {
        LOG_DEBUG << "#";

        beginResetModel();
        {
            m_ArtworksSnapshot.clear();
        }
        endResetModel();
    }

    void ArtworksViewModel::processArtworks(std::function<bool (const Artworks::ArtworkElement *element)> pred,
                                            std::function<void (size_t, Artworks::ArtworkMetadata *)> action) const {
        LOG_DEBUG << "#";

        auto &rawSnapshot = m_ArtworksSnapshot.getRawData();
        const size_t size = rawSnapshot.size();
        for (size_t i = 0; i < size; i++) {
            auto &item = rawSnapshot.at(i);
            const auto element = std::dynamic_pointer_cast<Artworks::ArtworkElement>(item);
            Q_ASSERT(element);

            if (pred(element.get())) {
                action(i, item->getArtworkMetadata());
            }
        }
    }

    void ArtworksViewModel::processArtworksEx(std::function<bool (const Artworks::ArtworkElement *element)> pred,
                                              std::function<bool (size_t, Artworks::ArtworkMetadata *)> action) const {
        LOG_DEBUG << "#";
        bool canContinue = false;

        auto &rawSnapshot = m_ArtworksSnapshot.getRawData();
        const size_t size = rawSnapshot.size();
        for (size_t i = 0; i < size; i++) {
            auto &locker = rawSnapshot.at(i);
            auto element = std::dynamic_pointer_cast<Artworks::ArtworkElement>(locker);
            Q_ASSERT(element);

            if (pred(element.get())) {
                canContinue = action(i, locker->getArtworkMetadata());

                if (!canContinue) { break; }
            }
        }
    }

    int ArtworksViewModel::rowCount(const QModelIndex &parent) const {
        Q_UNUSED(parent);
        return (int)m_ArtworksSnapshot.size();
    }

    QVariant ArtworksViewModel::data(const QModelIndex &index, int role) const {
        int row = index.row();
        if (row < 0 || row >= (int)m_ArtworksSnapshot.size()) { return QVariant(); }

        auto &item = m_ArtworksSnapshot.at(row);
        auto *artwork = item->getArtworkMetadata();

        switch (role) {
        case FilepathRole: return artwork->getFilepath();
        case IsSelectedRole: return accessItem(row)->getIsSelected();
        case HasVectorAttachedRole: {
            auto *imageArtwork = dynamic_cast<Artworks::ImageArtwork*>(artwork);
            return (imageArtwork != nullptr) && (imageArtwork->hasVectorAttached());
        }
        case ThumbnailPathRole: return artwork->getThumbnailPath();
        case IsVideoRole: {
            auto *videoArtwork = dynamic_cast<Artworks::VideoArtwork*>(artwork);
            return videoArtwork != nullptr;
        }
        default: return QVariant();
        }
    }

    QHash<int, QByteArray> ArtworksViewModel::roleNames() const {
        QHash<int, QByteArray> names = QAbstractListModel::roleNames();
        names[FilepathRole] = "filepath";
        names[IsSelectedRole] = "isselected";
        names[HasVectorAttachedRole] = "hasvectorattached";
        names[ThumbnailPathRole] = "thumbpath";
        names[IsVideoRole] = "isvideo";
        return names;
    }

    bool ArtworksViewModel::removeUnavailableItems() {
        LOG_DEBUG << "#";

        bool anyUnavailable = false;
        std::vector<int> indicesToRemove;
        const size_t size = m_ArtworksSnapshot.size();

        for (size_t i = 0; i < size; i++) {
            auto *artwork = m_ArtworksSnapshot.get(i);

            if (artwork->isUnavailable()) {
                indicesToRemove.push_back((int)i);
                anyUnavailable = true;
            }
        }

        if (anyUnavailable) {
            LOG_INFO << "Found" << indicesToRemove.size() << "unavailable item(s)";
            removeItems(Helpers::IndicesRanges(indicesToRemove));

            if (m_ArtworksSnapshot.empty()) {
                emit requestCloseWindow();
            }

            emit artworksCountChanged();
        }

        return anyUnavailable;
    }

    void ArtworksViewModel::removeInnerItem(int row) {
        m_ArtworksSnapshot.remove(row);
    }
}

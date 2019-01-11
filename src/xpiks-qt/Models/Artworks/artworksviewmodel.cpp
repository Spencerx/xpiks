/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "artworksviewmodel.h"

#include <QAbstractListModel>
#include <QByteArray>
#include <QVector>
#include <QtDebug>
#include <QtGlobal>

#include "Artworks/artworkelement.h"
#include "Artworks/artworkmetadata.h"
#include "Artworks/artworkssnapshot.h"
#include "Artworks/imageartwork.h"
#include "Artworks/videoartwork.h"  // IWYU pragma: keep
#include "Common/abstractlistmodel.h"
#include "Common/logging.h"
#include "Helpers/cpphelpers.h"
#include "Helpers/indicesranges.h"

class QModelIndex;

namespace Models {
    ArtworksViewModel::ArtworksViewModel(QObject *parent):
        AbstractListModel(parent)
    {
    }

    void ArtworksViewModel::handleMessage(UnavailableFilesMessage const &) {
        LOG_DEBUG << "#";
        removeUnavailableItems();
    }

    void ArtworksViewModel::setArtworks(const Artworks::ArtworksSnapshot &snapshot) {
        LOG_INFO << snapshot.size() << "artworks";
        if (snapshot.empty()) { return; }

        using namespace Artworks;
        auto elements = Helpers::map<std::shared_ptr<ArtworkMetadata>, std::shared_ptr<ArtworkElement>>(
                            snapshot.getRawData(),
                            [](const std::shared_ptr<ArtworkMetadata> &artwork) {
            return std::make_shared<ArtworkElement>(artwork);
        });

        beginResetModel();
        {
            m_ArtworksElements.swap(elements);
        }
        endResetModel();

        emit artworksCountChanged();
    }

    int ArtworksViewModel::getSelectedArtworksCount() const {
        int selectedCount = 0;
        size_t size = m_ArtworksElements.size();
        for (size_t i = 0; i < size; i++) {
            if (getIsSelected(i)) {
                selectedCount++;
            }
        }

        return selectedCount;
    }

    void ArtworksViewModel::resetModel() {
        LOG_DEBUG << "#";

        beginResetModel();
        {
            m_ArtworksElements.clear();
        }
        endResetModel();
    }

    void ArtworksViewModel::setArtworkSelected(int index, bool value) {
        if (index < 0 || index >= (int)m_ArtworksElements.size()) {
            return;
        }

        setIsSelected(index, value);

        QModelIndex qIndex = this->index(index);
        emit dataChanged(qIndex, qIndex, QVector<int>() << IsSelectedRole);
        emit selectedArtworksCountChanged();
    }

    void ArtworksViewModel::unselectAllItems() {
        const size_t size = m_ArtworksElements.size();
        for (size_t i = 0; i < size; i++) {
            setIsSelected(i, false);
        }

        emit dataChanged(this->index(0), this->index(rowCount() - 1), QVector<int>() << IsSelectedRole);
    }

    std::shared_ptr<Artworks::ArtworkElement> const &ArtworksViewModel::accessItem(size_t index) const {
        Q_ASSERT(index < m_ArtworksElements.size());
        return m_ArtworksElements[index];
    }

    bool ArtworksViewModel::getIsSelected(size_t i) const {
        return m_ArtworksElements.at(i)->getIsSelected();
    }

    void ArtworksViewModel::setIsSelected(size_t i, bool value) {
        m_ArtworksElements.at(i)->setIsSelected(value);
    }

    std::shared_ptr<Artworks::ArtworkMetadata> const &ArtworksViewModel::getArtwork(size_t i) const {
        Q_ASSERT((i >= 0) && (i < m_ArtworksElements.size()));
        return m_ArtworksElements[i]->getArtwork();
    }

    /*virtual*/
    bool ArtworksViewModel::doRemoveSelectedArtworks() {
        LOG_DEBUG << "#";

        const size_t size = m_ArtworksElements.size();
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

            if (m_ArtworksElements.empty()) {
                emit requestCloseWindow();
            }

            emit artworksCountChanged();
        }

        return anyItemToRemove;
    }

    Artworks::ArtworksSnapshot ArtworksViewModel::createSnapshot() {
        using ElementPtr = std::shared_ptr<Artworks::ArtworkElement>;
        using ArtworkPtr = std::shared_ptr<Artworks::ArtworkMetadata>;
        return Artworks::ArtworksSnapshot(
                    Helpers::map<ElementPtr, ArtworkPtr>(
                        m_ArtworksElements,
                        [](ElementPtr const &element){
                        return element->getArtwork();
                    }));
    }

    void ArtworksViewModel::processArtworks(std::function<bool (const std::shared_ptr<Artworks::ArtworkElement> &element)> pred,
                                            std::function<void (size_t, std::shared_ptr<Artworks::ArtworkMetadata> const &)> action) const {
        LOG_DEBUG << "#";

        const size_t size = m_ArtworksElements.size();
        for (size_t i = 0; i < size; i++) {
            auto &element = m_ArtworksElements.at(i);

            if (pred(element)) {
                action(i, element->getArtwork());
            }
        }
    }

    void ArtworksViewModel::processArtworksEx(std::function<bool (std::shared_ptr<Artworks::ArtworkElement> const &element)> pred,
                                              std::function<bool (size_t, std::shared_ptr<Artworks::ArtworkMetadata> const &)> action) const {
        LOG_DEBUG << "#";
        bool canContinue = false;

        const size_t size = m_ArtworksElements.size();
        for (size_t i = 0; i < size; i++) {
            auto &element = m_ArtworksElements.at(i);

            if (pred(element)) {
                canContinue = action(i, element->getArtwork());

                if (!canContinue) { break; }
            }
        }
    }

    int ArtworksViewModel::rowCount(const QModelIndex &parent) const {
        Q_UNUSED(parent);
        return (int)m_ArtworksElements.size();
    }

    QVariant ArtworksViewModel::data(const QModelIndex &index, int role) const {
        int row = index.row();
        if (row < 0 || row >= (int)m_ArtworksElements.size()) { return QVariant(); }

        auto &item = m_ArtworksElements.at(row);
        auto &artwork = item->getArtwork();

        switch (role) {
        case FilepathRole: return artwork->getFilepath();
        case IsSelectedRole: return accessItem(row)->getIsSelected();
        case HasVectorAttachedRole: {
            auto image = std::dynamic_pointer_cast<Artworks::ImageArtwork>(artwork);
            return (image != nullptr) && (image->hasVectorAttached());
        }
        case ThumbnailPathRole: return artwork->getThumbnailPath();
        case IsVideoRole: {
            auto video = std::dynamic_pointer_cast<Artworks::VideoArtwork>(artwork);
            return video != nullptr;
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
        const size_t size = m_ArtworksElements.size();

        for (size_t i = 0; i < size; i++) {
            auto &artwork = m_ArtworksElements[i]->getArtwork();

            if (artwork->isUnavailable()) {
                indicesToRemove.push_back((int)i);
                anyUnavailable = true;
            }
        }

        if (anyUnavailable) {
            LOG_INFO << "Found" << indicesToRemove.size() << "unavailable item(s)";
            removeItems(Helpers::IndicesRanges(indicesToRemove));

            if (m_ArtworksElements.empty()) {
                emit requestCloseWindow();
            }

            emit artworksCountChanged();
        }

        return anyUnavailable;
    }

    void ArtworksViewModel::removeInnerItem(int row) {
        m_ArtworksElements.erase(m_ArtworksElements.begin() + row);
    }
}

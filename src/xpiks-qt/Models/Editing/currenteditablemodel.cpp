/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "currenteditablemodel.h"
#include <Common/logging.h>
#include <Models/Artworks/artworkslistmodel.h>
#include <Models/Editing/combinedartworksmodel.h>
#include <Models/Editing/artworkproxymodel.h>

namespace Models {
    CurrentEditableModel::CurrentEditableModel(ArtworksListModel &artworksList,
                                               CombinedArtworksModel &combinedArtworksModel,
                                               ArtworkProxyModel &artworkProxyModel,
                                               QObject *parent) :
        QObject(parent),
        m_ArtworksListModel(artworksList),
        m_CombinedArtworksModel(combinedArtworksModel),
        m_ArtworkProxyModel(artworkProxyModel)
    {
        QObject::connect(&m_ArtworksListModel, &ArtworksListModel::currentArtworkChanged,
                         this, &CurrentEditableModel::onCurrentArtworkChanged);

        QObject::connect(&m_CombinedArtworksModel, &CombinedArtworksModel::currentEditableChanged,
                         this, &CurrentEditableModel::onCombinedEditableChanged);

        QObject::connect(&m_ArtworkProxyModel, &ArtworkProxyModel::currentEditableChanged,
                         this, &CurrentEditableModel::onProxyEditableChanged)
    }

    void CurrentEditableModel::clearCurrentItem() {
        LOG_DEBUG << "#";
        m_CurrentEditable.reset();
        emit currentEditableChanged();
    }

    void CurrentEditableModel::onCurrentArtworkChanged() {
        LOG_DEBUG << "#";
        m_CurrentEditable = std::move(m_ArtworksListModel.getCurrentEditable());
    }

    void CurrentEditableModel::onCombinedEditableChanged() {
        m_CurrentEditable = std::move(m_CombinedArtworksModel.getCurrentEditable());
    }

    void CurrentEditableModel::onProxyEditableChanged() {
        m_CurrentEditable = std::move(m_ArtworkProxyModel.getCurrentEditable());
    }
}

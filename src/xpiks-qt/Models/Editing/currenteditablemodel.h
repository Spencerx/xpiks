/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef CURRENTEDITABLEMODEL_H
#define CURRENTEDITABLEMODEL_H

#include <QObject>
#include <memory>
#include "icurrenteditable.h"

namespace Models {
    class ArtworksListModel;

    class CurrentEditableModel : public QObject
    {
        Q_OBJECT
        Q_PROPERTY(bool isAvailable READ getIsAvailable NOTIFY currentEditableChanged)

    public:
        explicit CurrentEditableModel(ArtworksListModel &artworksList, QObject *parent = 0);

    public:
        bool getIsAvailable() const { return m_CurrentEditable.operator bool(); }
        std::shared_ptr<Models::ICurrentEditable> getCurrentEditable() const { return m_CurrentEditable; }

    public:
        void clearCurrentItem();

    signals:
        void currentEditableChanged();

    public slots:
        void onCurrentArtworkChanged();

    private:
        std::shared_ptr<Models::ICurrentEditable> m_CurrentEditable;
        ArtworksListModel &m_ArtworksListModel;
    };
}

#endif // CURRENTEDITABLEMODEL_H

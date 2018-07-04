/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef ARTWORKSVIEWMODEL_H
#define ARTWORKSVIEWMODEL_H

#include <vector>
#include <functional>
#include <Common/abstractlistmodel.h>
#include <Artworks/artworkelement.h>
#include <Artworks/artworkmetadata.h>
#include <Helpers/ifilenotavailablemodel.h>
#include <Artworks/artworkssnapshot.h>

namespace Models {
    class ArtworksViewModel:
            public Common::AbstractListModel,
            public Helpers::IFileNotAvailableModel
    {
        Q_OBJECT
        Q_PROPERTY(int artworksCount READ getArtworksCount NOTIFY artworksCountChanged)
        Q_PROPERTY(int selectedArtworksCount READ getSelectedArtworksCount NOTIFY selectedArtworksCountChanged)

        enum ArtworksViewModel_Roles {
            FilepathRole = Qt::UserRole + 1,
            IsSelectedRole,
            HasVectorAttachedRole,
            ThumbnailPathRole,
            IsVideoRole
        };

    public:
        ArtworksViewModel(QObject *parent=NULL);
        virtual ~ArtworksViewModel() { }

    protected:
        virtual void setArtworks(Artworks::ArtworksSnapshot &&snapshot);

    public:
        int getArtworksCount() const { return (int)m_ArtworksSnapshot.size(); }
        int getSelectedArtworksCount() const;

    public:
        Q_INVOKABLE void setArtworkSelected(int index, bool value);
        Q_INVOKABLE void removeSelectedArtworks() { doRemoveSelectedArtworks(); }
        Q_INVOKABLE void resetModel() { doResetModel(); }
        Q_INVOKABLE void unselectAllItems();

#ifdef CORE_TESTS
    public:
#else
    protected:
#endif
        Artworks::ArtworkElement *accessItem(size_t index) const;
        bool getIsSelected(size_t i) const;
        void setIsSelected(size_t i, bool value);

    protected:
        bool isEmpty() const { return m_ArtworksSnapshot.empty(); }
        Artworks::ArtworkMetadata *getArtworkMetadata(size_t i) const;
        virtual bool doRemoveSelectedArtworks();
        virtual void doResetModel();
        void processArtworks(std::function<bool (const Artworks::ArtworkElement *element)> pred,
                             std::function<void (size_t, ArtworkMetadata *)> action) const;
        void processArtworksEx(std::function<bool (const Artworks::ArtworkElement *element)> pred,
                               std::function<bool (size_t, Artworks::ArtworkMetadata *)> action) const;

    protected:
        const Artworks::ArtworksSnapshot &getSnapshot() const { return m_ArtworksSnapshot; }

    signals:
        void artworksCountChanged();
        void selectedArtworksCountChanged();
        void requestCloseWindow();

        // QAbstractItemModel interface
    public:
        virtual int rowCount(const QModelIndex &parent=QModelIndex()) const override;
        virtual QVariant data(const QModelIndex &index, int role) const override;
        virtual QHash<int, QByteArray> roleNames() const override;

        // IFileNotAvailableModel interface
    public:
        virtual bool removeUnavailableItems() override;

        // AbstractListModel interface
    protected:
        virtual void removeInnerItem(int row) override;

    private:
        Artworks::ArtworksSnapshot m_ArtworksSnapshot;
    };
}

#endif // ARTWORKSVIEWMODEL_H

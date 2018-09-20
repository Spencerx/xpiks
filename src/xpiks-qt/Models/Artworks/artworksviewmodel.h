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
#include <Artworks/artworkssnapshot.h>
#include <Common/messages.h>
#include <Common/types.h>

namespace Models {
    using UnavailableFilesMessage = Common::NamedType<int, Common::MessageType::UnavailableFiles>;

    class ArtworksViewModel:
            public Common::AbstractListModel,
            public Common::MessagesTarget<UnavailableFilesMessage>
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

    public:
        virtual void handleMessage(UnavailableFilesMessage const &message) override;

    protected:
        virtual void setArtworks(const Artworks::ArtworksSnapshot &snapshot);

    public:
        int getArtworksCount() const { return (int)m_ArtworksElements.size(); }
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
        const std::shared_ptr<Artworks::ArtworkElement> &accessItem(size_t index) const;
        bool getIsSelected(size_t i) const;
        void setIsSelected(size_t i, bool value);

    protected:
        bool isEmpty() const { return m_ArtworksElements.empty(); }
        std::shared_ptr<Artworks::ArtworkMetadata> const &getArtwork(size_t i) const;
        virtual bool doRemoveSelectedArtworks();
        virtual void doResetModel();
        Artworks::ArtworksSnapshot createSnapshot();
        void processArtworks(std::function<bool (std::shared_ptr<Artworks::ArtworkElement> const &element)> pred,
                             std::function<void (size_t, std::shared_ptr<Artworks::ArtworkMetadata> const &)> action) const;
        void processArtworksEx(std::function<bool (std::shared_ptr<Artworks::ArtworkElement> const &element)> pred,
                               std::function<bool (size_t, std::shared_ptr<Artworks::ArtworkMetadata> const &)> action) const;

    signals:
        void artworksCountChanged();
        void selectedArtworksCountChanged();
        void requestCloseWindow();

        // QAbstractItemModel interface
    public:
        virtual int rowCount(const QModelIndex &parent=QModelIndex()) const override;
        virtual QVariant data(const QModelIndex &index, int role) const override;
        virtual QHash<int, QByteArray> roleNames() const override;

    public:
        virtual bool removeUnavailableItems();

        // AbstractListModel interface
    protected:
        virtual void removeInnerItem(int row) override;

    private:
        std::vector<std::shared_ptr<Artworks::ArtworkElement>> m_ArtworksElements;
    };
}

#endif // ARTWORKSVIEWMODEL_H

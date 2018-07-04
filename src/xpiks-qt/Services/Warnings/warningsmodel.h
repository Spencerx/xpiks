/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef WARNINGSMODEL_H
#define WARNINGSMODEL_H

#include <QSortFilterProxyModel>
#include <QStringList>
#include <QVector>
#include <Models/Artworks/artworkslistmodel.h>

namespace Warnings {
    class WarningsSettingsModel;

    class WarningsModel: public QSortFilterProxyModel
    {
        Q_OBJECT
        Q_PROPERTY(int warningsCount READ getWarningsCount NOTIFY warningsCountChanged)
        Q_PROPERTY(int minKeywordsCount READ getMinKeywordsCount NOTIFY warningsSettingsUpdated)
    public:
        WarningsModel(Models::ArtworksListModel &artworksListModel,
                      WarningsSettingsModel &warningsSettings,
                      QObject *parent=0);

    public:
        int getWarningsCount() const { return rowCount(); }
        int getMinKeywordsCount() const;

    public:
        Q_INVOKABLE void setShowSelected() { m_ShowOnlySelected = true; update(); }
        Q_INVOKABLE void resetShowSelected() { m_ShowOnlySelected = false; update(); }
        Q_INVOKABLE QStringList describeWarnings(int index) const;
        Q_INVOKABLE void update();
        Q_INVOKABLE int getOriginalIndex(int index) const;
        Q_INVOKABLE void processPendingUpdates();

    signals:
        void warningsCountChanged();
        void warningsSettingsUpdated();

    public slots:
        void onWarningsCouldHaveChanged(size_t originalIndex);
        void onWarningsUpdateRequired();

    private slots:
        void sourceRowsRemoved(QModelIndex,int,int);
        void sourceRowsInserted(QModelIndex,int,int);
        void sourceModelReset();

    private:
        enum WarningsModel_Roles {
            WarningsRole = Models::ArtworksListModel::RolesNumber + 1
        };

        // QSortFilterProxyModel interface
    protected:
        virtual bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

    public:
        virtual QVariant data(const QModelIndex &index, int role) const override;
        virtual QHash<int, QByteArray> roleNames() const override;

    private:
        WarningsSettingsModel &m_WarningsSettingsModel;
        Models::ArtworksListModel &m_ArtworksListModel;
        QVector<int> m_PendingUpdates;
        bool m_ShowOnlySelected;
    };
}

#endif // WARNINGSMODEL_H

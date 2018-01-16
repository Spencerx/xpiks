/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef TABSMODEL_H
#define TABSMODEL_H

#include <QAbstractListModel>
#include <QSortFilterProxyModel>
#include <QString>
#include <QVector>
#include <QSet>
#include <vector>
#include <utility>

#define FILES_FOLDERS_TAB_ID 3
#define QUICKBUFFER_TAB_ID 5
#define TRANSLATOR_TAB_ID 7

namespace QMLExtensions {
    class TabsModel : public QAbstractListModel
    {
        Q_OBJECT
        Q_PROPERTY(int tabsCount READ getTabsCount NOTIFY tabsCountChanged)
    public:
        explicit TabsModel(QObject *parent = 0);

    private:
        enum TabsModel_Roles {
            TabIconPathRole = Qt::UserRole + 1
            ,TabComponentPathRole
            ,ExternalTabIDRole
#ifdef QT_DEBUG
            ,CacheTagRole
#endif
        };

    public:
        struct CachedTab {
            CachedTab(unsigned int cacheTag, int tabIndex):
                m_CacheTag(cacheTag),
                m_TabIndex(tabIndex)
            { }

            unsigned int m_CacheTag;
            int m_TabIndex;
        };

    private:
        struct TabModel {
            QString m_TabIconPath;
            QString m_TabComponentPath;
            int m_ExternalTabID;
            int m_InternalTabID;
            unsigned int m_CacheTag;
            bool m_IsSystemTab;
        };

        // QAbstractItemModel interface
    public:
        virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
        virtual QVariant data(const QModelIndex &index, int role) const override;
        virtual QHash<int, QByteArray> roleNames() const override;

    signals:
        void tabRemoved();
        void cacheRebuilt();
        void tabsCountChanged();

    public:
        int getTabsCount() const { return m_TabsList.size(); }
        void addSystemTab(int systemTabID, const QString &iconPath, const QString &componentPath);
        void addPluginTab(int tabID, const QString &iconPath, const QString &componentPath);
        bool removePluginTab(int index);
        int findPluginTabIndexByID(int tabID);
        int findSystemTabIndexByID(int tabID);
        bool isTabActive(int index);
        bool activateSystemTab(int systemTabID);
        void activateTab(int index);
        void escalateTab(int index);
        bool touchTab(int index);
        TabModel &getTab(int index);
        void updateCache();
        int getMostRecentIndex() const;

    private:
        void recacheTab(int index);
        void addTab(int externalID, const QString &iconPath, const QString &componentPath);
        void rebuildCache();
        void updateActiveTabs();

    private:
        QVector<TabModel> m_TabsList;
        std::vector<CachedTab> m_LRUcache;
        QSet<int> m_ActiveTabs;
    };

    class DependentTabsModel: public QSortFilterProxyModel
    {
        Q_OBJECT
    public:
        explicit DependentTabsModel(QObject *parent = 0);

    public:
        Q_INVOKABLE void openTab(int index);
        Q_INVOKABLE int getIndex(int index) { return getOriginalIndex(index); }

    public slots:
        void onInvalidateRequired();

    protected:
        virtual void doOpenTab(int index) = 0;
        TabsModel *getTabsModel() const;
        int getOriginalIndex(int index) const;
    };

    class ActiveTabsModel: public DependentTabsModel
    {
        Q_OBJECT
    public:
        explicit ActiveTabsModel(QObject *parent = 0);

    public:
        Q_INVOKABLE void reactivateMostRecentTab();

    public slots:
        void onInactiveTabOpened(int index);

    signals:
        void tabActivateRequested(int originalTabIndex);

        // QSortFilterProxyModel interface
    protected:
        virtual bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

        // DependentTabsModel interface
    protected:
        virtual void doOpenTab(int index) override;
    };

    class InactiveTabsModel: public DependentTabsModel
    {
        Q_OBJECT
    public:
        explicit InactiveTabsModel(QObject *parent = 0);

    signals:
        void tabOpened(int index);

        // QSortFilterProxyModel interface
    protected:
        virtual bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
        virtual bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;

        // DependentTabsModel interface
    protected:
        virtual void doOpenTab(int index) override;
    };
}

#endif // TABSMODEL_H

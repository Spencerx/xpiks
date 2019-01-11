/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "tabsmodel.h"

#include <algorithm>
#include <cstddef>

#include <QAbstractItemModel>
#include <QByteArray>
#include <QList>
#include <QtAlgorithms>
#include <QtDebug>
#include <QtGlobal>

#include "Common/logging.h"

namespace QMLExtensions {
    bool compareCachePairs(const TabsModel::CachedTab &left, const TabsModel::CachedTab &right) {
        return left.m_CacheTag > right.m_CacheTag;
    }

    TabsModel::TabsModel(QObject *parent) :
        QAbstractListModel(parent)
    {
    }

    int TabsModel::rowCount(const QModelIndex &parent) const {
        Q_UNUSED(parent);
        return m_TabsList.size();
    }

    QVariant TabsModel::data(const QModelIndex &index, int role) const {
        int row = index.row();
        if ((row < 0) || (row >= m_TabsList.size())) {
            return QVariant();
        }

        auto &tab = m_TabsList[row];

        switch (role) {
        case TabIconPathRole: return tab.m_TabIconPath;
        case TabComponentPathRole: return tab.m_TabComponentPath;
        case ExternalTabIDRole: return tab.m_ExternalTabID;
        case CacheTagRole: return tab.m_CacheTag;
        default: return QVariant();
        }
    }

    QHash<int, QByteArray> TabsModel::roleNames() const {
        auto roles = QAbstractListModel::roleNames();
        roles[TabIconPathRole] = "tabicon";
        roles[TabComponentPathRole] = "tabcomponent";
        roles[ExternalTabIDRole] = "tabid";
        roles[CacheTagRole] = "cachetag";
        return roles;
    }

    void TabsModel::addSystemTab(int systemTabID, const QString &iconPath, const QString &componentPath) {
        LOG_INFO << iconPath << componentPath;
        addTab(0, iconPath, componentPath);
        auto &tab = m_TabsList.last();
        tab.m_IsSystemTab = true;
        tab.m_InternalTabID = systemTabID;
    }

    void TabsModel::addPluginTab(int tabID, const QString &iconPath, const QString &componentPath) {
        LOG_INFO << iconPath << componentPath;
        addTab(tabID, iconPath, componentPath);
    }

    bool TabsModel::removePluginTab(int index) {
        LOG_INFO << index;

        bool success = false;

        if ((0 <= index) && (index < m_TabsList.size())) {
            Q_ASSERT(!m_TabsList[index].m_IsSystemTab);
            if (!m_TabsList[index].m_IsSystemTab) {
                beginRemoveRows(QModelIndex(), index, index);
                {
                    m_TabsList.removeAt(index);
                }
                endRemoveRows();

                updateCache();
                emit tabRemoved();
                emit tabsCountChanged();
                success = true;
            } else {
                LOG_WARNING << "Attempt to remove system tab";
            }
        }

        return success;
    }

    int TabsModel::findPluginTabIndexByID(int tabID) {
        int index = -1;
        const int size = m_TabsList.size();
        for (int i = 0; i < size; i++) {
            auto &tab = m_TabsList[i];
            if (tab.m_IsSystemTab) { continue; }

            if (tab.m_ExternalTabID == tabID) {
                index = i;
                break;
            }
        }

        return index;
    }

    int TabsModel::findSystemTabIndexByID(int tabID) {
        int index = -1;
        const int size = m_TabsList.size();
        for (int i = 0; i < size; i++) {
            auto &tab = m_TabsList[i];
            if (!tab.m_IsSystemTab) { continue; }

            if (tab.m_InternalTabID == tabID) {
                index = i;
                break;
            }
        }

        return index;
    }

    bool TabsModel::isTabActive(int index) {
        bool found = m_ActiveTabs.contains(index);
        return found;
    }

    bool TabsModel::activateSystemTab(int systemTabID) {
        bool activated = false;

        const int index = findSystemTabIndexByID(systemTabID);
        const int mostRecentIndex = getMostRecentIndex();

        if ((index >= 0) && (mostRecentIndex != index)) {
            activateTab(index);
            activated = true;
        }

        return activated;
    }

    void TabsModel::activateTab(int index) {
        LOG_INFO << index;
        if ((index < 0) || (index >= m_TabsList.size())) { return; }

        auto tabsIndices = m_ActiveTabs.toList();
        Q_ASSERT(tabsIndices.size() == 3);
        qSort(tabsIndices.begin(), tabsIndices.end(), [this](int left, int right) -> bool {
            return this->m_TabsList[left].m_CacheTag < this->m_TabsList[right].m_CacheTag;
        });

        m_TabsList[index].m_CacheTag = m_TabsList[tabsIndices[2]].m_CacheTag;
        touchTab(index);
    }

    void TabsModel::escalateTab(int index) {
        activateTab(index);
    }

    bool TabsModel::touchTab(int index) {
        bool success = false;
        LOG_INFO << index;

        if ((0 <= index) && (index < m_TabsList.size())) {
            auto &tab = m_TabsList[index];
            tab.m_CacheTag++;
            recacheTab(index);
            updateActiveTabs();
            emit dataChanged(this->index(index), this->index(index));
            success = true;
        }

        return success;
    }

    TabsModel::TabModel &TabsModel::getTab(int index) {
        Q_ASSERT((0 <= index) && (index < m_TabsList.size()));
        return m_TabsList[index];
    }

    void TabsModel::updateCache() {
         rebuildCache();
         updateActiveTabs();
    }

    int TabsModel::getMostRecentIndex() const {
        int index = 0;
        unsigned int maxTag = 0;

        const int size = m_TabsList.size();
        for (int i = 0; i < size; i++) {
            if (m_ActiveTabs.contains(i)) {
                unsigned int tag = m_TabsList[i].m_CacheTag;
                if (tag > maxTag) {
                    index = i;
                    maxTag = tag;
                }
            }
        }

        return index;
    }

    void TabsModel::recacheTab(int index) {
        Q_ASSERT((0 <= index) && (index < m_TabsList.size()));
        auto &tab = m_TabsList[index];

        CachedTab cachedTab(tab.m_CacheTag, index);
        m_LRUcache.insert(
                    std::upper_bound(m_LRUcache.begin(), m_LRUcache.end(), cachedTab, compareCachePairs),
                    cachedTab);

        if (m_LRUcache.size() > 10*(size_t)m_TabsList.size()) {
            rebuildCache();
        }
    }

    void TabsModel::addTab(int externalID, const QString &iconPath, const QString &componentPath) {
        int lastIndex = m_TabsList.length();
        beginInsertRows(QModelIndex(), lastIndex, lastIndex);
        {
            m_TabsList.push_back(TabModel());
            auto &item = m_TabsList[lastIndex];
            item.m_TabIconPath = iconPath;
            item.m_TabComponentPath = componentPath;
            item.m_ExternalTabID = externalID;
            item.m_CacheTag = 0;
            item.m_IsSystemTab = false;
        }
        endInsertRows();

        emit tabsCountChanged();
    }

    void TabsModel::rebuildCache() {
        LOG_DEBUG << "#";

        const int size = m_TabsList.size();

        m_LRUcache.clear();
        m_LRUcache.reserve(size);

        for (int i = 0; i < size; ++i) {
            auto &item = m_TabsList[i];
            m_LRUcache.emplace_back(item.m_CacheTag, i);
        }

        std::sort(m_LRUcache.begin(), m_LRUcache.end(), compareCachePairs);
        emit cacheRebuilt();
    }

    void TabsModel::updateActiveTabs() {
        QSet<int> activeTabs;

        for (auto &item: m_LRUcache) {
            if (!activeTabs.contains(item.m_TabIndex)) {
                activeTabs.insert(item.m_TabIndex);
            }

            if (activeTabs.count() == 3) {
                break;
            }
        }

        m_ActiveTabs.swap(activeTabs);
    }

    DependentTabsModel::DependentTabsModel(QObject *parent):
        QSortFilterProxyModel(parent)
    { }

    void DependentTabsModel::openTab(int index) {
        doOpenTab(index);
    }

    void DependentTabsModel::onInvalidateRequired() {
        LOG_DEBUG << "#";
        invalidateFilter();
    }

    TabsModel *DependentTabsModel::getTabsModel() const {
        QAbstractItemModel *sourceItemModel = sourceModel();
        TabsModel *tabsModel = dynamic_cast<TabsModel *>(sourceItemModel);
        Q_ASSERT(tabsModel != nullptr);
        return tabsModel;
    }

    int DependentTabsModel::getOriginalIndex(int index) const {
        LOG_INFO << index;
        QModelIndex originalIndex = mapToSource(this->index(index, 0));
        int row = originalIndex.row();
        return row;
    }

    ActiveTabsModel::ActiveTabsModel(QObject *parent):
        DependentTabsModel(parent)
    { }

    void ActiveTabsModel::reactivateMostRecentTab() {
        auto *tabsModel = getTabsModel();
        int index = tabsModel->getMostRecentIndex();
        emit tabActivateRequested(index);
    }

    void ActiveTabsModel::onInactiveTabOpened(int index) {
        LOG_INFO << index;
        invalidateFilter();
        emit tabActivateRequested(index);
    }

    bool ActiveTabsModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const {
        Q_UNUSED(source_parent);
        auto *tabsModel = getTabsModel();
        bool isActiveTab = tabsModel->isTabActive(source_row);
        return isActiveTab;
    }

    void ActiveTabsModel::doOpenTab(int index) {
        LOG_INFO << index;
        int originalIndex = getOriginalIndex(index);
        auto *tabsModel = getTabsModel();

        tabsModel->activateTab(originalIndex);
    }

    InactiveTabsModel::InactiveTabsModel(QObject *parent):
        DependentTabsModel(parent)
    { }

    bool InactiveTabsModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const {
        Q_UNUSED(source_parent);
        auto *tabsModel = getTabsModel();
        bool isActiveTab = tabsModel->isTabActive(source_row);
        return !isActiveTab;
    }

    bool InactiveTabsModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const {
        auto *tabsModel = getTabsModel();

        if (tabsModel->rowCount(QModelIndex()) <= 6) {
            return DependentTabsModel::lessThan(source_left, source_right);
        }

        auto &leftTab = tabsModel->getTab(source_left.row());
        auto &rightTab = tabsModel->getTab(source_right.row());
        bool less = leftTab.m_CacheTag < rightTab.m_CacheTag;
        return less;
    }

    void InactiveTabsModel::doOpenTab(int index) {
        LOG_INFO << index;
        int originalIndex = getOriginalIndex(index);
        auto *tabsModel = getTabsModel();

        tabsModel->escalateTab(originalIndex);
        invalidateFilter();
        emit tabOpened(originalIndex);
    }
}

/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "uimanager.h"
#include <QQuickTextDocument>
#include <QScreen>
#include <QQmlEngine>
#include <Common/defines.h>
#include <Artworks/basicmetadatamodel.h>
#include <Models/settingsmodel.h>
#include <Models/Artworks/artworkslistmodel.h>
#include <Helpers/loghighlighter.h>

#define MAX_SAVE_PAUSE_RESTARTS 5

#define DEFAULT_ARTWORK_EDIT_RIGHT_PANE_WIDTH 300

#define DEFAULT_APP_WIDTH 900
#define DEFAULT_APP_HEIGHT 725
#define DEFAULT_APP_POSITION -1

namespace Models {
    UIManager::UIManager(Common::ISystemEnvironment &environment,
                         QMLExtensions::ColorsModel &colorsModel,
                         SettingsModel &settingsModel,
                         QObject *parent) :
        QObject(parent),
        Common::DelayedActionEntity(500, MAX_SAVE_PAUSE_RESTARTS),
        m_State("uimanager", environment),
        m_ColorsModel(colorsModel),
        m_SettingsModel(settingsModel),
        m_TabID(42),
        m_ScreenDPI(96.0)
    {
        QObject::connect(&m_SettingsModel, &Models::SettingsModel::keywordSizeScaleChanged,
                         this, &UIManager::keywordHeightChanged);

        m_ActiveTabs.setSourceModel(&m_TabsModel);
        m_InactiveTabs.setSourceModel(&m_TabsModel);

        QObject::connect(&m_TabsModel, &QMLExtensions::TabsModel::tabRemoved, &m_ActiveTabs, &QMLExtensions::ActiveTabsModel::onInvalidateRequired);
        QObject::connect(&m_TabsModel, &QMLExtensions::TabsModel::cacheRebuilt, &m_ActiveTabs, &QMLExtensions::ActiveTabsModel::onInvalidateRequired);

        QObject::connect(&m_TabsModel, &QMLExtensions::TabsModel::tabRemoved, &m_InactiveTabs, &QMLExtensions::InactiveTabsModel::onInvalidateRequired);
        QObject::connect(&m_TabsModel, &QMLExtensions::TabsModel::cacheRebuilt, &m_InactiveTabs, &QMLExtensions::InactiveTabsModel::onInvalidateRequired);

        QObject::connect(&m_InactiveTabs, &QMLExtensions::InactiveTabsModel::tabOpened, &m_ActiveTabs, &QMLExtensions::ActiveTabsModel::onInactiveTabOpened);
    }

    void UIManager::setScreenDpi(double value) {
        LOG_INFO << value;
        if (value != m_ScreenDPI) {
            m_ScreenDPI = value;
            emit screenDpiChanged();
        }
    }

    double UIManager::getKeywordHeight() const {
        double keywordScale = m_SettingsModel.getKeywordSizeScale();
        double height = 20.0 * keywordScale + (keywordScale - 1.0) * 10.0;
        return height;
    }

    QObject *UIManager::retrieveTabsModel(int tabID) {
        QObject *model = nullptr;
        if (m_TabIDsToModel.contains(tabID)) {
            model = m_TabIDsToModel[tabID];
            QQmlEngine::setObjectOwnership(model, QQmlEngine::CppOwnership);
        }

        return model;
    }

    void UIManager::sync() {
        LOG_DEBUG << "#";
        m_State.sync();
    }

    int UIManager::getArtworkEditRightPaneWidth() {
        return m_State.getInt(Constants::artworkEditRightPaneWidth, DEFAULT_ARTWORK_EDIT_RIGHT_PANE_WIDTH);
    }

    void UIManager::setArtworkEditRightPaneWidth(int value) {
        const int current = getArtworkEditRightPaneWidth();
        if (current != value) {
            m_State.setValue(Constants::artworkEditRightPaneWidth, value);
            justChanged();
            emit artworkEditRightPaneWidthChanged();
        }
    }

    int UIManager::getAppWidth(int defaultWidth)  {
        return m_State.getInt(Constants::appWindowWidth, defaultWidth);
    }

    void UIManager::setAppWidth(int width) {
        LOG_DEBUG << width;
        m_State.setValue(Constants::appWindowWidth, width);
        justChanged();
    }

    int UIManager::getAppHeight(int defaultHeight) {
        return m_State.getInt(Constants::appWindowHeight, defaultHeight);
    }

    void UIManager::setAppHeight(int height) {
        LOG_DEBUG << height;
        m_State.setValue(Constants::appWindowHeight, height);
        justChanged();
    }

    int UIManager::getAppPosX(int defaultPosX, int maxPosX) {
        Q_ASSERT(defaultPosX < maxPosX);
        int posX = m_State.getInt(Constants::appWindowX, defaultPosX);
        if ((posX < 0) || (posX >= maxPosX)) { posX = defaultPosX; }
        return posX;
    }

    void UIManager::setAppPosX(int x) {
        LOG_DEBUG << x;
        m_State.setValue(Constants::appWindowX, x);
        justChanged();
    }

    int UIManager::getAppPosY(int defaultPosY, int maxPosY) {
        Q_ASSERT(defaultPosY < maxPosY);
        int posY = m_State.getInt(Constants::appWindowY, defaultPosY);
        if ((posY < 0) || (posY >= maxPosY)) { posY = defaultPosY; }
        return posY;
    }

    void UIManager::setAppPosY(int y) {
        LOG_DEBUG << y;
        m_State.setValue(Constants::appWindowY, y);
        justChanged();
    }

    void UIManager::initDescriptionHighlighting(QObject *basicModelObject, QQuickTextDocument *document) {
        Artworks::BasicMetadataModel *basicModel = qobject_cast<Artworks::BasicMetadataModel*>(basicModelObject);
        if (basicModel != nullptr) {
            SpellCheck::SpellCheckInfo &info = basicModel->getSpellCheckInfo();
            info.createHighlighterForDescription(document->textDocument(), &m_ColorsModel, basicModel);
            basicModel->notifyDescriptionSpellingChanged();
        }
    }

    void UIManager::initTitleHighlighting(QObject *basicModelObject, QQuickTextDocument *document) {
        Artworks::BasicMetadataModel *basicModel = qobject_cast<Artworks::BasicMetadataModel*>(basicModelObject);
        if (basicModel != nullptr) {
            SpellCheck::SpellCheckInfo &info = basicModel->getSpellCheckInfo();
            info.createHighlighterForTitle(document->textDocument(), &m_ColorsModel, basicModel);
            basicModel->notifyDescriptionSpellingChanged();
        }
    }

    void UIManager::initLogsHighlighting(QQuickTextDocument *document) {
        LOG_DEBUG << "#";
        Helpers::LogHighlighter *highlighter = new Helpers::LogHighlighter(m_ColorsModel,
                                                                           document->textDocument());
        Q_UNUSED(highlighter);
    }

    void UIManager::activateQuickBufferTab() {
        if (m_TabsModel.activateSystemTab(QUICKBUFFER_TAB_ID)) {
            m_ActiveTabs.reactivateMostRecentTab();
        }
    }

    void UIManager::addSystemTab(int systemTabID, const QString &tabIconComponent, const QString &tabComponent) {
        LOG_INFO << "icon" << tabIconComponent << "contents" << tabComponent;
        m_TabsModel.addSystemTab(systemTabID, tabIconComponent, tabComponent);
        generateNextTabID();
    }

    int UIManager::addPluginTab(int pluginID, const QString &tabIconComponent, const QString &tabComponent, QObject *tabModel) {
        LOG_INFO << "plugin" << pluginID << "icon" << tabIconComponent << "contents" << tabComponent;

        int id = generateNextTabID();

        m_TabsModel.addPluginTab(id, tabIconComponent, tabComponent);

        if (tabModel != nullptr) {
            Q_ASSERT(!m_TabIDsToModel.contains(id));
            m_TabIDsToModel.insert(id, tabModel);
        }

        if (!m_PluginIDToTabIDs.contains(pluginID)) {
            m_PluginIDToTabIDs.insert(pluginID, QSet<int>());
        }

        m_PluginIDToTabIDs[pluginID].insert(id);

        LOG_INFO << "Added plugin tab with ID" << id;

        return id;
    }

    bool UIManager::removePluginTab(int pluginID, int tabID) {
        LOG_INFO << "plugin" << pluginID << "tab" << tabID;
        bool success = false;

        do {
            if (!m_PluginIDToTabIDs.contains(pluginID)) {
                break;
            }

            auto &tabsSet = m_PluginIDToTabIDs[pluginID];
            if (!tabsSet.contains(tabID)) {
                break;
            }

            int index = m_TabsModel.findPluginTabIndexByID(tabID);
            if (index == -1) {
                break;
            }

            tabsSet.remove(tabID);
            m_TabsModel.removePluginTab(index);
            m_TabIDsToModel.remove(tabID);

            LOG_INFO << "Plugin's tab" << tabID << "removed";
            success = true;
        } while(false);

        if (!success) {
            LOG_WARNING << "Failed to remove plugin tab" << tabID;
        }

        return success;
    }

    void UIManager::initializeSystemTabs() {
        m_TabsModel.updateCache();

        m_TabsModel.touchTab(0);
        m_TabsModel.touchTab(1);
        m_TabsModel.touchTab(2);

        m_TabsModel.touchTab(0);
    }

    void UIManager::initialize() {
        LOG_DEBUG << "#";
        m_State.init();
    }

    void UIManager::resetWindowSettings() {
        // resetting position in settings is pretty useless because
        // we will overwrite them on Xpiks exit. But anyway for the future...
        m_State.setValue(Constants::appWindowHeight, DEFAULT_APP_HEIGHT);
        m_State.setValue(Constants::appWindowWidth, DEFAULT_APP_WIDTH);
        m_State.setValue(Constants::appWindowX, DEFAULT_APP_POSITION);
        m_State.setValue(Constants::appWindowY, DEFAULT_APP_POSITION);

        m_State.setValue(Constants::artworkEditRightPaneWidth, DEFAULT_ARTWORK_EDIT_RIGHT_PANE_WIDTH);

        justChanged();
    }

    void UIManager::onScreenDpiChanged(qreal someDpi) {
        Q_UNUSED(someDpi);
        QScreen *screen = qobject_cast<QScreen*>(sender());
        updateDpi(screen);
    }

    void UIManager::onScreenChanged(QScreen *screen) {
        updateDpi(screen);

        if (screen != nullptr) {
            QObject::connect(screen, &QScreen::logicalDotsPerInchChanged,
                             this, &Models::UIManager::onScreenDpiChanged);
            QObject::connect(screen, &QScreen::physicalDotsPerInchChanged,
                             this, &Models::UIManager::onScreenDpiChanged);
        }
    }

    void UIManager::updateDpi(QScreen *screen) {
        if (screen != nullptr) {
            LOG_INFO << "Device pixel ratio:" << screen->devicePixelRatio();
            LOG_INFO << "Logical dots per inch:" << screen->logicalDotsPerInch();

            qreal dpi;
#ifdef Q_OS_WIN
            dpi = screen->logicalDotsPerInch() * screen->devicePixelRatio();
#else
            dpi = screen->physicalDotsPerInch() * screen->devicePixelRatio();
#endif
            setScreenDpi(dpi);
        }
    }

    void UIManager::doOnTimer() {
        m_State.sync();
    }
}

/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef UIMANAGER_H
#define UIMANAGER_H

#include <memory>
#include <QSet>
#include <QHash>
#include <QObject>
#include <QString>
#include <QStringList>
#include "../QuickBuffer/icurrenteditable.h"
#include "../Models/artworkelement.h"
#include "../QMLExtensions/tabsmodel.h"
#include "../Common/statefulentity.h"
#include "../Helpers/constants.h"
#include "../Common/delayedactionentity.h"
#include "../Common/isystemenvironment.h"

class QScreen;

namespace Models {
    class ArtworkMetadata;
    class ArtworkProxyBase;
    class SettingsModel;

    class UIManager:
            public QObject,
            public Common::DelayedActionEntity

    {
        Q_OBJECT
        Q_PROPERTY(bool hasCurrentEditable READ getHasCurrentEditable NOTIFY currentEditableChanged)
        Q_PROPERTY(double keywordHeight READ getKeywordHeight NOTIFY keywordHeightChanged)
        Q_PROPERTY(int artworkEditRightPaneWidth READ getArtworkEditRightPaneWidth WRITE setArtworkEditRightPaneWidth NOTIFY artworkEditRightPaneWidthChanged)
        Q_PROPERTY(double screenDpi READ getScreenDpi NOTIFY screenDpiChanged)

    public:
        explicit UIManager(Common::ISystemEnvironment &environment, Models::SettingsModel *settingsModel,
                           QObject *parent = 0);

    private:
        int generateNextTabID() { int id = m_TabID++; return id; }
        void setScreenDpi(double value);

    public:
        double getScreenDpi() const { return m_ScreenDPI; }
        void initTabs();
        bool getHasCurrentEditable() const { return m_CurrentEditable.operator bool(); }
        double getKeywordHeight() const;
        std::shared_ptr<QuickBuffer::ICurrentEditable> getCurrentEditable() const { return m_CurrentEditable; }

    public:
        void registerCurrentItem(std::shared_ptr<QuickBuffer::ICurrentEditable> &currentItem);

    public:
        QMLExtensions::TabsModel *getTabsModel() { return &m_TabsModel; }
        QMLExtensions::ActiveTabsModel *getActiveTabs() { return &m_ActiveTabs; }
        QMLExtensions::InactiveTabsModel *getInactiveTabs() { return &m_InactiveTabs; }

    public:
        Q_INVOKABLE void clearCurrentItem();
        Q_INVOKABLE QObject *retrieveTabsModel(int tabID);
        Q_INVOKABLE void sync();

    public:
        int getArtworkEditRightPaneWidth();
        void setArtworkEditRightPaneWidth(int value);
        Q_INVOKABLE int getAppWidth(int defaultWidth);
        Q_INVOKABLE void setAppWidth(int width);
        Q_INVOKABLE int getAppHeight(int defaultHeight);
        Q_INVOKABLE void setAppHeight(int height);
        Q_INVOKABLE int getAppPosX(int defaultPosX, int maxPosX);
        Q_INVOKABLE void setAppPosX(int x);
        Q_INVOKABLE int getAppPosY(int defaultPosY, int maxPosY);
        Q_INVOKABLE void setAppPosY(int y);

    public:
        Q_INVOKABLE void activateQuickBufferTab();

    public:
        void addSystemTab(int systemTabID, const QString &tabIconComponent, const QString &tabComponent);
        int addPluginTab(int pluginID, const QString &tabIconComponent, const QString &tabComponent, QObject *tabModel);
        bool removePluginTab(int pluginID, int tabID);
        void initializeSystemTabs();
        void initialize();
        void resetWindowSettings();

    signals:
        void tabsListChanged();
        void tabsIconsChanged();
        void currentEditableChanged();
        void keywordHeightChanged(double value);
        void artworkEditRightPaneWidthChanged();
        void screenDpiChanged();

    public slots:
        void onScreenDpiChanged(qreal someDpi);
        void onScreenChanged(QScreen *screen);

    private:
        void updateDpi(QScreen *screen);

        // DelayedActionEntity implementation
    protected:
        virtual void doKillTimer(int timerId) override { this->killTimer(timerId); }
        virtual int doStartTimer(int interval, Qt::TimerType timerType) override { return this->startTimer(interval, timerType); }
        virtual void doOnTimer() override;
        virtual void timerEvent(QTimerEvent *event) override { onQtTimer(event); }
        virtual void callBaseTimer(QTimerEvent *event) override { QObject::timerEvent(event); }

    private:
        Common::StatefulEntity m_State;
        Models::SettingsModel *m_SettingsModel;
        QMLExtensions::TabsModel m_TabsModel;
        QMLExtensions::ActiveTabsModel m_ActiveTabs;
        QMLExtensions::InactiveTabsModel m_InactiveTabs;
        std::shared_ptr<QuickBuffer::ICurrentEditable> m_CurrentEditable;
        QHash<int, QSet<int> > m_PluginIDToTabIDs;
        QHash<int, QObject*> m_TabIDsToModel;
        volatile int m_TabID;
        double m_ScreenDPI;
    };
}

#endif // UIMANAGER_H

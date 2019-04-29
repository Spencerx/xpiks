/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef UIMANAGER_H
#define UIMANAGER_H

#include <QHash>
#include <QObject>
#include <QString>
#include <Qt>
#include <QtGlobal>

#include "Common/delayedactionentity.h"
#include "Common/statefulentity.h"
#include "QMLExtensions/tabsmodel.h"

class QQuickTextDocument;
class QScreen;
class QTimerEvent;
template <class T> class QSet;

namespace Common {
    class ISystemEnvironment;
}

namespace QMLExtensions {
    class ColorsModel;
}

namespace Models {
    class SettingsModel;

    class UIManager:
            public QObject,
            public Common::DelayedActionEntity

    {
        Q_OBJECT
        Q_PROPERTY(double keywordHeight READ getKeywordHeight NOTIFY keywordHeightChanged)
        Q_PROPERTY(int artworkEditRightPaneWidth READ getArtworkEditRightPaneWidth WRITE setArtworkEditRightPaneWidth NOTIFY artworkEditRightPaneWidthChanged)
        Q_PROPERTY(double screenDpi READ getScreenDpi NOTIFY screenDpiChanged)

    public:
        explicit UIManager(Common::ISystemEnvironment &environment,
                           QMLExtensions::ColorsModel &colorsModel,
                           SettingsModel &settingsModel,
                           QObject *parent = nullptr);

    private:
        int generateNextTabID() { int id = m_TabID++; return id; }
        void setScreenDpi(double value);

    public:
        double getScreenDpi() const { return m_ScreenDPI; }
        void initTabs();
        double getKeywordHeight() const;

    public:
        QMLExtensions::TabsModel *getTabsModel() { return &m_TabsModel; }
        QMLExtensions::ActiveTabsModel *getActiveTabs() { return &m_ActiveTabs; }
        QMLExtensions::InactiveTabsModel *getInactiveTabs() { return &m_InactiveTabs; }

    public:
        Q_INVOKABLE QObject *retrieveTabsModel(int tabID);
        Q_INVOKABLE void sync();

    public:
        int getArtworkEditRightPaneWidth();
        void setArtworkEditRightPaneWidth(int value);

    public:
        Q_INVOKABLE int getAppWidth(int defaultWidth);
        Q_INVOKABLE void setAppWidth(int width);
        Q_INVOKABLE int getAppHeight(int defaultHeight);
        Q_INVOKABLE void setAppHeight(int height);
        Q_INVOKABLE int getAppPosX(int defaultPosX, int maxPosX);
        Q_INVOKABLE void setAppPosX(int x);
        Q_INVOKABLE int getAppPosY(int defaultPosY, int maxPosY);
        Q_INVOKABLE void setAppPosY(int y);

    public:
        Q_INVOKABLE void initDescriptionHighlighting(QObject *basicModelObject, QQuickTextDocument *document);
        Q_INVOKABLE void initTitleHighlighting(QObject *basicModelObject, QQuickTextDocument *document);
        Q_INVOKABLE void initLogsHighlighting(QQuickTextDocument *document);
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
        QMLExtensions::ColorsModel &m_ColorsModel;
        Models::SettingsModel &m_SettingsModel;
        QMLExtensions::TabsModel m_TabsModel;
        QMLExtensions::ActiveTabsModel m_ActiveTabs;
        QMLExtensions::InactiveTabsModel m_InactiveTabs;
        QHash<int, QSet<int> > m_PluginIDToTabIDs;
        QHash<int, QObject*> m_TabIDsToModel;
        volatile int m_TabID;
        double m_ScreenDPI;
    };
}

#endif // UIMANAGER_H

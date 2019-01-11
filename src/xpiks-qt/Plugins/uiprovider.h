/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef UIPROVIDER_H
#define UIPROVIDER_H

#include <QHash>
#include <QObject>
#include <QQmlComponent>
#include <QString>
#include <QVector>

class QQmlEngine;
class QQuickItem;
class QUrl;

namespace Models {
    class UIManager;
}

namespace Plugins {
    class UIProvider : public QObject
    {
        Q_OBJECT
    public:
        UIProvider(Models::UIManager &uiManager, QObject *parent=0);

    public:
        Models::UIManager &getUIManager() const { return m_UiManager; }

    public:
        void setQmlEngine(QQmlEngine *engine) { m_QmlEngine = engine; }
        void setRoot(QQuickItem *root) { m_Root = root; }

    public:
        void closeAllDialogs();

        // IUIProvider interface
    public:
        void openDialog(const QUrl &rcPath, const QHash<QString, QObject*> &contextModels = QHash<QString, QObject*>());

    private slots:
        void viewStatusChanged(QQmlComponent::Status status);
        void componentDestroyed(QObject *object);
        void contextDestroyed(QObject *object);

    private:
        QQmlEngine *m_QmlEngine;
        QQuickItem *m_Root;
        QVector<QObject*> m_OpenedDialogs;
        Models::UIManager &m_UiManager;
    };
}

#endif // UIPROVIDER_H

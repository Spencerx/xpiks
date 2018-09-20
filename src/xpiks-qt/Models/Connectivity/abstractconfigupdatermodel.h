/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef ABSTRACTUPDATERCONFIGMODEL_H
#define ABSTRACTUPDATERCONFIGMODEL_H

#include <QSet>
#include <QJsonDocument>
#include <QFileInfo>
#include <Helpers/comparevaluesjson.h>
#include <Helpers/remoteconfig.h>
#include <Helpers/localconfig.h>
#include <Helpers/jsonhelper.h>

namespace Connectivity {
    class IRequestsService;
}

namespace Models {
    class AbstractConfigUpdaterModel: public QObject, public Helpers::CompareValuesJson
    {
        Q_OBJECT
    public:
        AbstractConfigUpdaterModel(const QString &localPath,
                                   const QString &remoteResource,
                                   bool forceOverwrite,
                                   bool memoryOnly, QObject *parent=nullptr);
        virtual ~AbstractConfigUpdaterModel() {}

    public:
        void initializeConfigs(Connectivity::IRequestsService &requestsService);
        const Helpers::LocalConfig &getLocalConfig() const { return m_LocalConfig; }
        Helpers::LocalConfig &getLocalConfig() { return m_LocalConfig; }

#if defined(INTEGRATION_TESTS) || defined(UI_TESTS)
        void setRemoteOverride(const QString &localPath) {
            Q_ASSERT(QFileInfo(localPath).exists());
            m_RemoteOverrideLocalPath = localPath;
        }
#endif

    private slots:
        void remoteConfigArrived();

    protected:
        virtual void processRemoteConfig(const QJsonDocument &remoteDocument, bool overwriteLocal);
        virtual bool processLocalConfig(const QJsonDocument &document) = 0;
        virtual void processMergedConfig(const QJsonDocument &document) { Q_UNUSED(document); }

    private:
        virtual void initRemoteConfig(Connectivity::IRequestsService &requestsService);
        virtual void initLocalConfig();

    private:
        Helpers::RemoteConfig m_RemoteConfig;
        Helpers::LocalConfig m_LocalConfig;
        bool m_ForceOverwrite;

#if defined(INTEGRATION_TESTS) || defined(UI_TESTS)
        bool m_MemoryOnly;
        QString m_RemoteOverrideLocalPath;
#endif
    };
}
#endif // QABSTRACTUPDATERCONFIGMODEL_H

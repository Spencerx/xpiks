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
#include "../Helpers/comparevaluesjson.h"
#include "../Helpers/remoteconfig.h"
#include "../Helpers/localconfig.h"
#include "../Helpers/jsonhelper.h"
#include "../Common/baseentity.h"

namespace Models {
    class AbstractConfigUpdaterModel: public QObject, public Helpers::CompareValuesJson, public Common::BaseEntity
    {
        Q_OBJECT
    public:
        AbstractConfigUpdaterModel(bool forceOverwrite, QObject *parent=nullptr);
        virtual ~AbstractConfigUpdaterModel() {}

    public:
        void initializeConfigs(const QString &configUrl, const QString &filePath);
        const Helpers::LocalConfig &getLocalConfig() const { return m_LocalConfig; }
        Helpers::LocalConfig &getLocalConfig() { return m_LocalConfig; }

#ifdef INTEGRATION_TESTS
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
        virtual void initRemoteConfig(const QString &configUrl);
        virtual void initLocalConfig(const QString &filePath);

    private:
        Helpers::RemoteConfig m_RemoteConfig;
        Helpers::LocalConfig m_LocalConfig;
        bool m_ForceOverwrite;

#ifdef INTEGRATION_TESTS
        QString m_RemoteOverrideLocalPath;
#endif
    };
}
#endif // QABSTRACTUPDATERCONFIGMODEL_H

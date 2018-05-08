/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "abstractconfigupdatermodel.h"
#include "../Models/settingsmodel.h"
#include "../Commands/commandmanager.h"
#include "../Connectivity/requestsservice.h"

namespace Models {
    AbstractConfigUpdaterModel::AbstractConfigUpdaterModel(const QString &localPath,
                                                           const QString &remoteResource,
                                                           bool forceOverwrite,
                                                           bool memoryOnly,
                                                           QObject *parent):
        QObject(parent)
        ,m_RemoteConfig(remoteResource, this)
        ,m_LocalConfig(localPath, memoryOnly)
        ,m_ForceOverwrite(forceOverwrite)
  #ifdef INTEGRATION_TESTS
        ,m_MemoryOnly(memoryOnly)
  #endif
    {
        QObject::connect(&m_RemoteConfig, &Helpers::RemoteConfig::configArrived,
                         this, &AbstractConfigUpdaterModel::remoteConfigArrived);
    }

    void AbstractConfigUpdaterModel::initializeConfigs() {
        LOG_DEBUG << "#";

        initLocalConfig();
        initRemoteConfig();
    }

    void AbstractConfigUpdaterModel::remoteConfigArrived() {
        LOG_DEBUG << "#";
        const QJsonDocument &remoteDocument = m_RemoteConfig.getConfig();
        processRemoteConfig(remoteDocument, m_ForceOverwrite);
    }

    void AbstractConfigUpdaterModel::processRemoteConfig(const QJsonDocument &remoteDocument, bool overwriteLocal) {
        LOG_DEBUG << "#";
        QJsonDocument &localDocument = m_LocalConfig.getConfig();
        Helpers::mergeJson(remoteDocument, localDocument, overwriteLocal, *this);
        m_LocalConfig.save();

        processMergedConfig(localDocument);
    }

    void AbstractConfigUpdaterModel::initRemoteConfig() {
#ifdef INTEGRATION_TESTS
        if (!m_RemoteOverrideLocalPath.isEmpty()) {
            LOG_DEBUG << "Using remote override" << m_RemoteOverrideLocalPath;
            Helpers::LocalConfig m_RemoteOverrideConfig(m_RemoteOverrideLocalPath, m_MemoryOnly);
            m_RemoteOverrideConfig.initialize();

            const QJsonDocument &localDocument = m_RemoteOverrideConfig.getConfig();
            processRemoteConfig(localDocument, m_ForceOverwrite);
        } else
#endif
        {
            Q_ASSERT(m_CommandManager != NULL);
            Connectivity::RequestsService *requestsService = m_CommandManager->getRequestsService();
            requestsService->receiveConfig(&m_RemoteConfig);
        }
    }

    void AbstractConfigUpdaterModel::initLocalConfig() {
        m_LocalConfig.initialize();
        const QJsonDocument &localDocument = m_LocalConfig.getConfig();
        processLocalConfig(localDocument);
    }
}

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

#include <QtDebug>

#include "Common/logging.h"
#include "Connectivity/irequestsservice.h"
#include "Helpers/jsonhelper.h"
#include "Helpers/localconfig.h"
#include "Helpers/remoteconfig.h"

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
  #if defined(INTEGRATION_TESTS) || defined(UI_TESTS)
      ,m_MemoryOnly(memoryOnly)
  #endif
    {
        QObject::connect(&m_RemoteConfig, &Helpers::RemoteConfig::configArrived,
                         this, &AbstractConfigUpdaterModel::remoteConfigArrived);
    }

    void AbstractConfigUpdaterModel::initializeConfigs(Connectivity::IRequestsService &requestsService) {
        LOG_DEBUG << "#";

        initLocalConfig();
        initRemoteConfig(requestsService);
    }

    void AbstractConfigUpdaterModel::remoteConfigArrived() {
        LOG_DEBUG << "#";
        const QJsonDocument &remoteDocument = m_RemoteConfig.getConfig();
        processRemoteConfig(remoteDocument, m_ForceOverwrite);
    }

    void AbstractConfigUpdaterModel::processRemoteConfig(const QJsonDocument &remoteDocument, bool overwriteLocal) {
        LOG_DEBUG << "#";
        QJsonDocument document = m_LocalConfig.readConfig();
        Helpers::mergeJson(remoteDocument, document, overwriteLocal, *this);
        m_LocalConfig.writeConfig(document);

        processMergedConfig(document);
    }

    void AbstractConfigUpdaterModel::initRemoteConfig(Connectivity::IRequestsService &requestsService) {
#if defined(INTEGRATION_TESTS) || defined(UI_TESTS)
        if (!m_RemoteOverrideLocalPath.isEmpty()) {
            LOG_DEBUG << "Using remote override" << m_RemoteOverrideLocalPath;
            QJsonDocument localDocument = Helpers::LocalConfig(m_RemoteOverrideLocalPath).readConfig();
            processRemoteConfig(localDocument, m_ForceOverwrite);
        } else
#endif
        {
            requestsService.receiveConfig(m_RemoteConfig);
        }
    }

    void AbstractConfigUpdaterModel::initLocalConfig() {
        QJsonDocument localDocument = m_LocalConfig.readConfig();
        processLocalConfig(localDocument);
    }
}

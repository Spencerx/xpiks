/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SESSIONMANAGER_H
#define SESSIONMANAGER_H

#include <QJsonObject>
#include <QMutex>
#include <memory>
#include "../Helpers/localconfig.h"
#include "../Common/isystemenvironment.h"
#include "../Helpers/jsonobjectmap.h"

namespace MetadataIO {
    class ArtworkSessionSnapshot;
    class SessionSnapshot;
}

namespace Models {
    class ArtworkMetadata;
    class ArtworksRepository;
}

namespace Models {
    class SessionManager: public QObject
    {
        Q_OBJECT
    public:
        SessionManager(Common::ISystemEnvironment &environment);
        virtual ~SessionManager() {}

    public:
        bool save(std::vector<std::shared_ptr<MetadataIO::ArtworkSessionSnapshot> > &filesSnapshot,
                        const QStringList &directoriesSnapshot);
        int restoreSession(Models::ArtworksRepository &artworksRepository);

    public:
        bool getIsEmergencyRestore() const { return m_EmergencyRestore; }

#ifdef INTEGRATION_TESTS
        int itemsCount() const { return m_LastSavedFilesCount; }
        void clearSession();
    private:
        int m_LastSavedFilesCount = 0;
#endif

    private:
        Helpers::LocalConfig m_Config;
        QMutex m_Mutex;
        bool m_EmergencyRestore;
    };
}

#endif // SESSIONMANAGER_H

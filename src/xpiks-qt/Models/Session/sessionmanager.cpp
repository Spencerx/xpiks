/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QDir>
#include <QJsonArray>
#include <Artworks/artworkssnapshot.h>
#include <Filesystem/filescollection.h>
#include "sessionmanager.h"

#define SESSION_FILE "session.json"

#define OPENED_FILES_KEY "openedFiles"
#define OPENED_DIRECTORIES_KEY "openedDirectories"
#define FILE_KEY "file"
#define VECTOR_KEY "vector"
#define DIRECTORY_PATH_KEY "dirpath"

namespace Models {    
    std::shared_ptr<Helpers::JsonObjectMap> serializeSnapshot(std::vector<std::shared_ptr<Artworks::ArtworkSessionSnapshot> > &filesSnapshot,
                                                              const QStringList &directoriesSnapshot) {
        LOG_INFO << filesSnapshot.size() << "artwork(s)" << directoriesSnapshot.size() << "directory(ies)";

        auto sessionMap = std::make_shared<Helpers::JsonObjectMap>();
        QJsonArray filesList;

        for (auto &item: filesSnapshot) {
            QJsonObject jsonObject;

            auto &filePath = item->getArtworkFilePath();
            jsonObject.insert(FILE_KEY, filePath);

            auto &vectorPath = item->getAttachedVectorPath();
            if (!vectorPath.isEmpty()) {
                jsonObject.insert(VECTOR_KEY, vectorPath);
            }

            filesList.append(jsonObject);
        }

        sessionMap->setValue(OPENED_FILES_KEY, filesList);

        QJsonArray directoriesList;
        for (auto item: directoriesSnapshot) {
            QJsonObject jsonObject;
            jsonObject.insert(DIRECTORY_PATH_KEY, item);
            directoriesList.append(jsonObject);
        }

        sessionMap->setValue(OPENED_DIRECTORIES_KEY, directoriesList);
        return sessionMap;
    }

    void parseFiles(const std::shared_ptr<Helpers::JsonObjectMap> &sessionMap, QStringList &filenames, QStringList &vectors) {
        if (!sessionMap->containsValue(OPENED_FILES_KEY)) { return; }

        QJsonValue openedFilesValue = sessionMap->value(OPENED_FILES_KEY);
        if (!openedFilesValue.isArray()) { return; }

        QJsonArray filesArray = openedFilesValue.toArray();

        if (filesArray.isEmpty()) {
            LOG_DEBUG << "The session does not contain standalone files";
            return;
        }

        filenames.reserve(filesArray.size());
        vectors.reserve(filesArray.size()/2);

        for (const auto &item: filesArray) {
            QJsonObject obj = item.toObject();
            QString filePath = obj.value(FILE_KEY).toString().trimmed();
            if (filePath.isEmpty()) { continue; }

            filenames.push_back(filePath);

            QString vectorPath = obj.value(VECTOR_KEY).toString().trimmed();
            if (!vectorPath.isEmpty()) {
                vectors.append(vectorPath);
            }
        }
    }

    void parseDirectories(const std::shared_ptr<Helpers::JsonObjectMap> &sessionMap, QStringList &fullDirectories) {
        if (!sessionMap->containsValue(OPENED_DIRECTORIES_KEY)) { return; }

        QJsonValue openedDirsValue = sessionMap->value(OPENED_DIRECTORIES_KEY);
        if (!openedDirsValue.isArray()) { return; }

        QJsonArray dirsArray = openedDirsValue.toArray();

        if (dirsArray.isEmpty()) {
            LOG_DEBUG << "The session does not contain full directories";
            return;
        }

        fullDirectories.reserve(dirsArray.size());

        for (const auto &item: dirsArray) {
            QJsonObject obj = item.toObject();
            QString dirPath = obj.value(DIRECTORY_PATH_KEY).toString().trimmed();
            if (dirPath.isEmpty()) { continue; }

            fullDirectories.append(dirPath);
        }
    }

    SessionManager::SessionManager(Common::ISystemEnvironment &environment):
        QObject(),
        m_Config(environment.path({SESSION_FILE}),
                 environment.getIsInMemoryOnly()),
        m_EmergencyRestore(environment.getIsRecoveryMode())
    {
        if (environment.getIsInMemoryOnly()) {
            LOG_WARNING << "Session restoration will not work in read-only mode";
        }
    }

    bool SessionManager::save(std::unique_ptr<Artworks::SessionSnapshot> &sessionSnapshot) {
        auto sessionMap = serializeSnapshot(sessionSnapshot->getSnapshot(), sessionSnapshot->getDirectoriesSnapshot());
        bool success = false;

        {
            QMutexLocker locker(&m_Mutex);
            Q_UNUSED(locker);

            do {
                success = m_Config.writeMap(sessionMap);
#ifdef INTEGRATION_TESTS
                m_LastSavedFilesCount = success ? (int)filesSnapshot.size() : m_LastSavedFilesCount;
#endif
            } while (false);
        }

        return success;
    }

    SessionManager::SessionTuple SessionManager::restoreSession() {
        LOG_DEBUG << "#";

        QMutexLocker locker(&m_Mutex);
        Q_UNUSED(locker);

        QStringList filenames, vectors, fullDirectories;
        std::shared_ptr<Helpers::JsonObjectMap> sessionMap = m_Config.readMap();

        parseFiles(sessionMap, filenames, vectors);
        parseDirectories(sessionMap, fullDirectories);

        if (filenames.empty()) {
            LOG_INFO << "Session was empty";
            return SessionTuple(
                        std::make_shared<Filesystem::FilesCollection>(QStringList()),
                        QStringList());
        }

        return SessionTuple(
                    std::make_shared<Filesystem::FilesCollection>(
                        std::initializer_list<QStringList>({filenames, vectors})),
                    fullDirectories);
    }

#ifdef INTEGRATION_TESTS
    void SessionManager::clearSession() {
        std::vector<std::shared_ptr<Artworks::ArtworkSessionSnapshot> > emptyFiles;
        QStringList emptyDirs;
        bool cleared = save(emptyFiles, emptyDirs);
        Q_ASSERT(cleared);
    }
#endif
}

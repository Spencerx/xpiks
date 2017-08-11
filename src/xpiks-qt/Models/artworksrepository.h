/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2017 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef ARTWORKSDIRECTORIES_H
#define ARTWORKSDIRECTORIES_H

#include <QAbstractListModel>
#include <QStringList>
#include <QList>
#include <QPair>
#include <QSet>
#include <QTimer>
#include "../Common/abstractlistmodel.h"
#include "../Common/baseentity.h"
#include <QFileSystemWatcher>

namespace Models {
    class ArtworksRepository : public Common::AbstractListModel, public Common::BaseEntity {
        Q_OBJECT
        Q_PROPERTY(int artworksSourcesCount READ getArtworksSourcesCount NOTIFY artworksSourcesCountChanged)
    public:
        ArtworksRepository(QObject *parent = 0);
        virtual ~ArtworksRepository() {}

    public:
        enum ArtworksRepository_Roles {
            PathRole = Qt::UserRole + 1,
            UsedImagesCountRole,
            IsSelectedRole
        };

    public:
        void updateCountsForExistingDirectories();
        void cleanupEmptyDirectories();
        void resetLastUnavailableFilesCount() { m_LastUnavailableFilesCount=0; }
        void stopListeningToUnavailableFiles();

    public:
        bool beginAccountingFiles(const QStringList &items);
        void endAccountingFiles(bool filesWereAccounted);

    public:
        virtual int getNewDirectoriesCount(const QStringList &items) const;
        int getNewFilesCount(const QStringList &items) const;
        int getArtworksSourcesCount() const { return m_DirectoriesList.length(); }
        bool canPurgeUnavailableFiles() const { return m_UnavailableFiles.size() == m_LastUnavailableFilesCount; }

    signals:
        void artworksSourcesCountChanged();
        void fileChanged(const QString & path);
        void filesUnavailable();

#ifdef CORE_TESTS
    protected:
        void insertIntoUnavailable(const QString &value) { m_UnavailableFiles.insert(value); }
        const QSet<QString> &getFilesSet() const { return m_FilesSet; }
#endif

    public slots:
        void fileSelectedChanged(const QString &filepath, bool isSelected) { setFileSelected(filepath, isSelected); }

    private slots:
        void checkFileUnavailable(const QString & path);
        void onAvailabilityTimer();

    public:
        bool accountFile(const QString &filepath);
        void accountVector(const QString &vectorPath);
        bool removeFile(const QString &filepath, const QString &fileDirectory);
        void removeVector(const QString &vectorPath);
        void setFileSelected(const QString &filepath, bool selected);
        void purgeUnavailableFiles();
        void watchFilePaths(const QStringList &filePaths);
        void unwatchFilePaths(const QStringList &filePaths);
        void updateFilesCounts();

    private:
        void watchFilePath(const QString &filepath);

    public:
        const QString &getDirectory(int index) const { return m_DirectoriesList[index]; }
#ifdef CORE_TESTS
        int getFilesCountForDirectory(const QString &directory) const { return m_DirectoriesHash[directory]; }
        int getFilesCountForDirectory(int index) const { return m_DirectoriesHash[m_DirectoriesList[index]]; }
#endif
        bool isFileUnavailable(const QString &filepath) const;

#ifdef INTEGRATION_TESTS
        void resetEverything();
#endif

    public:
        virtual int rowCount(const QModelIndex & parent = QModelIndex()) const override;
        virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const override;

    protected:
        virtual QHash<int, QByteArray> roleNames() const override;

    protected:
        virtual void removeInnerItem(int index) override {
            QString directoryToRemove = m_DirectoriesList.takeAt(index);
            m_DirectoriesSelectedHash.remove(directoryToRemove);
            m_DirectoriesHash.remove(directoryToRemove);
            emit artworksSourcesCountChanged();
        }

        virtual bool checkFileExists(const QString &filename, QString &directory) const;

    private:
        QStringList m_DirectoriesList;
        QHash<QString, int> m_DirectoriesHash;
        QSet<QString> m_FilesSet;
        QHash<QString, int> m_DirectoriesSelectedHash;
        QFileSystemWatcher m_FilesWatcher;
        QTimer m_Timer;
        QSet<QString> m_UnavailableFiles;
        int m_LastUnavailableFilesCount;
    };
}

#endif // ARTWORKSDIRECTORIES_H

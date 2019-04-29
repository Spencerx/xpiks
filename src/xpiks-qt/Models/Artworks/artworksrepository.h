/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef ARTWORKSDIRECTORIES_H
#define ARTWORKSDIRECTORIES_H

#include <cstddef>
#include <memory>
#include <vector>

#include <QFileSystemWatcher>
#include <QHash>
#include <QModelIndex>
#include <QObject>
#include <QSet>
#include <QSortFilterProxyModel>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QVariant>
#include <Qt>
#include <QtGlobal>

#include "Artworks/artworkssnapshot.h"
#include "Common/abstractlistmodel.h"
#include "Common/flags.h"

class QByteArray;
class QModelIndex;

namespace Filesystem {
    class IFilesCollection;
}

namespace Models {
    class RecentDirectoriesModel;
    struct ArtworksRemoveResult;

    class ArtworksRepository : public Common::AbstractListModel {
        Q_OBJECT
    public:
        ArtworksRepository(RecentDirectoriesModel &recentDirectories, QObject *parent = nullptr);
        virtual ~ArtworksRepository() override {}

    public:
        enum ArtworksRepository_Roles {
            PathRole = Qt::UserRole + 1,
            UsedImagesCountRole,
            IsSelectedRole,
            IsRemovedRole,
            IsFullRole
        };

    public:
        struct RepoDir {
            RepoDir (QString absolutePath, qint64 id, int count):
                m_AbsolutePath(absolutePath), m_ID(id), m_FilesCount(count), m_DirectoryFlags(Common::DirectoryFlags::None)
            { }
            RepoDir() = default;

            inline bool getIsSelectedFlag() const { return Common::HasFlag(m_DirectoryFlags, Common::DirectoryFlags::IsSelected); }
            inline bool getAddedAsDirectoryFlag() const { return Common::HasFlag(m_DirectoryFlags, Common::DirectoryFlags::IsAddedAsDirectory); }
            inline bool getIsRemovedFlag() const { return Common::HasFlag(m_DirectoryFlags, Common::DirectoryFlags::IsRemoved); }

            inline void setIsSelectedFlag(bool value) { Common::ApplyFlag(m_DirectoryFlags, value, Common::DirectoryFlags::IsSelected); }
            inline void setAddedAsDirectoryFlag(bool value) { Common::ApplyFlag(m_DirectoryFlags, value, Common::DirectoryFlags::IsAddedAsDirectory); }
            inline void setIsRemovedFlag(bool value) { Common::ApplyFlag(m_DirectoryFlags, value, Common::DirectoryFlags::IsRemoved); }

            bool isValid() const { return m_FilesCount > 0; }

            QString m_AbsolutePath = QString("");
            qint64 m_ID = 0;
            int m_FilesCount = 0;
            Common::DirectoryFlags m_DirectoryFlags = Common::DirectoryFlags::None;
        };

    public:
        bool isEmpty(int index) const;
        void resetLastUnavailableFilesCount() { m_LastUnavailableFilesCount = 0; }
        void stopListeningToUnavailableFiles();

    public:
        virtual int getNewDirectoriesCount(const QStringList &items) const;
        int getNewFilesCount(const std::shared_ptr<Filesystem::IFilesCollection> &files) const;
        bool canPurgeUnavailableFiles() const { return m_UnavailableFiles.size() == m_LastUnavailableFilesCount; }
        bool isDirectorySelected(qint64 directoryID) const;

    signals:
        void refreshRequired();
        void filteringChanged();
        void artworksSourcesChanged();
        void fileChanged(const QString &path);
        void filesUnavailable();
        void selectionChanged();

#ifdef CORE_TESTS
    public:
        void removeItem(int index) { removeInnerItem(index); }
        int getFilesCountForDirectory(const QString &directory) const { size_t index; tryFindDirectory(directory, index); return m_DirectoriesList[index].m_FilesCount; }
        std::vector<RepoDir> &accessRepos() { return m_DirectoriesList; }

    protected:
        const QSet<QString> &getFilesSet() const { return m_FilesSet; }
        void setFullDirectory(int index) { m_DirectoriesList[index].setAddedAsDirectoryFlag(true); }
        void unsetFullDirectory(int index) { m_DirectoriesList[index].setAddedAsDirectoryFlag(false); }
#endif

#if defined(CORE_TESTS) || defined(INTEGRATION_TESTS)
    public:
        int getFilesCountForDirectory(int index) const { return m_DirectoriesList[index].m_FilesCount; }
        void insertIntoUnavailable(const QString &value) { m_UnavailableFiles.insert(value); }
        void notifyUnavailableFiles() { emit filesUnavailable(); }
#endif

    public slots:
        void onUndoStackEmpty();

    private slots:
        void checkFileUnavailable(const QString &path);
        void onAvailabilityTimer();

    public:
        Common::AccountFileFlags accountFile(const QString &filepath, qint64 &directoryID);
        Common::RemoveFileFlags removeFile(const QString &filepath, qint64 directoryID);
        void removeVector(const QString &vectorPath);
        void cleanupEmptyDirectories();
        void purgeUnavailableFiles();
        void watchFiles(const Artworks::ArtworksSnapshot &snapshot);
        void setFullDirectories(const QSet<qint64> &directoryIDs);
        void removeFiles(Artworks::ArtworksSnapshot const &snapshot, ArtworksRemoveResult &removeResult);
        void unwatchFilePaths(const QStringList &filePaths);
        void updateFilesCounts();
        void updateSelectedState();

    private:
        qint64 generateNextID() { qint64 id = m_LastID; m_LastID++; return id; }

    public:
        bool tryGetDirectoryPath(qint64 directoryID, QString &absolutePath) const;
        const QString &getDirectoryPath(int index) const { Q_ASSERT((0 <= index) && (index < (int)m_DirectoriesList.size())); return m_DirectoriesList[index].m_AbsolutePath; }
        qint64 getDirectoryID(int index) const { Q_ASSERT((0 <= index) && (index < (int)m_DirectoriesList.size())); return m_DirectoriesList[index].m_ID; }
        bool isFileUnavailable(const QString &filepath) const;

#if defined(INTEGRATION_TESTS) || defined(UI_TESTS)
        void resetEverything();
#endif
        QStringList retrieveFullDirectories() const;
        void restoreFullDirectories(const QStringList &directories);

    public:
        virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
        virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    public:
        QSet<qint64> consolidateSelectionForEmpty();
        void restoreDirectoriesSelection(const QSet<qint64> &selectedDirectories);
        void toggleDirectorySelected(size_t row);

    protected:
        virtual QHash<int, QByteArray> roleNames() const override;

    protected:
        virtual void removeInnerItem(int index) override;
        virtual bool checkFileExists(const QString &filename, QString &directory) const;

    public:
        bool unselectAllDirectories() { return setAllSelected(false); }
        bool selectAllDirectories() { return setAllSelected(true); }
        bool allAreSelected() const;
        size_t retrieveSelectedDirsCount() const;

    private:
        bool setDirectorySelected(size_t index, bool value);
        bool changeSelectedState(size_t row, bool newValue, bool oldValue);
        bool setAllSelected(bool value);
        bool tryFindDirectory(const QString &directoryPath, size_t &index) const;
        bool tryFindDirectoryByID(qint64 id, size_t &index) const;

    private:
        std::vector<RepoDir> m_DirectoriesList;
        RecentDirectoriesModel &m_RecentDirectories;
        QSet<QString> m_FilesSet;
        QFileSystemWatcher m_FilesWatcher;
        QTimer m_Timer;
        QSet<QString> m_UnavailableFiles;
        int m_LastUnavailableFilesCount;
        qint64 m_LastID;
    };

    class FilteredArtworksRepository: public QSortFilterProxyModel {
        Q_OBJECT
        Q_PROPERTY(int artworksSourcesCount READ getArtworksSourcesCount NOTIFY artworksSourcesCountChanged)

    public:
        FilteredArtworksRepository(ArtworksRepository &artworksRepository);

    public:
        int getArtworksSourcesCount() { return rowCount(); }

    public:
        void setArtworksRepository(ArtworksRepository &artworksRepository);

    public:
        Q_INVOKABLE int getOriginalIndex(int index);
        Q_INVOKABLE void selectDirectory(int row);

    protected:
        virtual bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

    signals:
        void artworksSourcesCountChanged();
        void directoriesFiltered();

    private slots:
        void onRefreshRequired();

    private:
        ArtworksRepository &m_ArtworksRepository;
    };
}

#endif // ARTWORKSDIRECTORIES_H

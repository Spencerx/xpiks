/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "artworksrepository.h"

#include <QByteArray>
#include <QDir>
#include <QFileInfo>
#include <QSet>
#include <QVector>
#include <QtDebug>

#include "Artworks/artworkmetadata.h"
#include "Artworks/artworkssnapshot.h"
#include "Artworks/imageartwork.h"
#include "Common/abstractlistmodel.h"
#include "Common/flags.h"
#include "Common/logging.h"
#include "Filesystem/ifilescollection.h"
#include "Helpers/indicesranges.h"
#include "Models/Artworks/artworkslistoperations.h"
#include "Models/Session/recentdirectoriesmodel.h"

class QModelIndex;

namespace Models {
    ArtworksRepository::ArtworksRepository(RecentDirectoriesModel &recentDirectories, QObject *parent) :
        AbstractListModel(parent),
        m_RecentDirectories(recentDirectories),
        m_LastUnavailableFilesCount(0),
        m_LastID(0)
    {
        QObject::connect(&m_FilesWatcher, &QFileSystemWatcher::fileChanged,
                         this, &ArtworksRepository::checkFileUnavailable);

        m_Timer.setInterval(4000); //4 sec
        m_Timer.setSingleShot(true); //single shot
        QObject::connect(&m_Timer, &QTimer::timeout, this, &ArtworksRepository::onAvailabilityTimer);
    }

    bool ArtworksRepository::isEmpty(int index) const {
        bool empty = false;
        if ((0 <= index) && (index < (int)m_DirectoriesList.size())) {
            empty = (m_DirectoriesList[index].m_FilesCount == 0);
        } else {
            empty = true;
        }
        return empty;
    }

    void ArtworksRepository::stopListeningToUnavailableFiles() {
        LOG_DEBUG << "#";

        QStringList files = m_FilesWatcher.files();
        if (!files.isEmpty()) {
            m_FilesWatcher.removePaths(files);
        }

        QStringList directories = m_FilesWatcher.directories();
        if (!directories.isEmpty()) {
            m_FilesWatcher.removePaths(directories);
        }
    }

    /*virtual */
    int ArtworksRepository::getNewDirectoriesCount(const QStringList &items) const {
        QSet<QString> filteredFiles;

        foreach (const QString &filepath, items) {
            if (!m_FilesSet.contains(filepath)) {
                filteredFiles.insert(filepath);
            }
        }

        QSet<QString> filteredDirectories;

        foreach (const QString &filepath, filteredFiles) {
            QString directory;
            if (checkFileExists(filepath, directory)) {
                filteredDirectories.insert(directory);
            }
        }

        int count = 0;
        size_t phony;
        foreach (const QString &directory, filteredDirectories) {
            if (!tryFindDirectory(directory, phony)) {
                count++;
            }
        }

        return count;
    }

    int ArtworksRepository::getNewFilesCount(const std::shared_ptr<Filesystem::IFilesCollection> &files) const {
        int count = 0;

        for (auto &file: files->getFiles()) {
            if (file.m_Type == Filesystem::ArtworkFileType::Vector) { continue; }

            if (!m_FilesSet.contains(file.m_Path)) {
                count++;
            }
        }

        return count;
    }

    bool ArtworksRepository::isDirectorySelected(qint64 directoryID) const {
        bool isSelected = false;

        size_t index = 0;
        if (tryFindDirectoryByID(directoryID, index)) {
            isSelected = m_DirectoriesList[index].getIsSelectedFlag();
        }

        return isSelected;
    }

    void ArtworksRepository::onUndoStackEmpty() {
        LOG_DEBUG << "#";
        cleanupEmptyDirectories();
        emit artworksSourcesChanged();
    }

    Common::AccountFileFlags ArtworksRepository::accountFile(const QString &filepath, qint64 &directoryID) {
        Common::AccountFileFlags flags = Common::AccountFileFlags::None;
        QString directoryPath;

        if (this->checkFileExists(filepath, directoryPath) &&
                !m_FilesSet.contains(filepath)) {
            int occurances = 0;
            size_t index = 0;
            bool alreadyExists = tryFindDirectory(directoryPath, index);
            if (!alreadyExists) {
                const qint64 id = generateNextID();
                LOG_INFO << "Adding new directory" << directoryPath << "with index" << m_DirectoriesList.size() << "and id" << id;
                m_DirectoriesList.emplace_back(directoryPath, id, 0);
                auto &item = m_DirectoriesList.back();
                item.setIsSelectedFlag(true);
                index = m_DirectoriesList.size() - 1;
                directoryID = id;
                Common::SetFlag(flags, Common::AccountFileFlags::FlagRepositoryCreated);
            } else {
                auto &item = m_DirectoriesList[index];
                occurances = item.m_FilesCount;
                directoryID = item.m_ID;
            }

            // watchFilePath(filepath);
            m_FilesSet.insert(filepath);
            auto &item = m_DirectoriesList[index];
            item.setIsRemovedFlag(false);
            item.m_FilesCount = occurances + 1;
            Common::SetFlag(flags, Common::AccountFileFlags::FlagRepositoryModified);
        }

        if (Common::HasFlag(flags, Common::AccountFileFlags::FlagRepositoryCreated)) {
            m_RecentDirectories.pushItem(directoryPath);
            const int lastIndex = (int)m_DirectoriesList.size() - 1;
            beginInsertRows(QModelIndex(), lastIndex, lastIndex);
            // ...
            endInsertRows();
            emit artworksSourcesChanged();
        }

        return flags;
    }

    Common::RemoveFileFlags ArtworksRepository::removeFile(const QString &filepath, qint64 directoryID) {
        Common::RemoveFileFlags result = Common::RemoveFileFlags::None;

        if (m_FilesSet.contains(filepath)) {
            size_t existingIndex = 0;
            if (tryFindDirectoryByID(directoryID, existingIndex)) {
                auto &item = m_DirectoriesList[existingIndex];
                item.m_FilesCount--;
                Common::SetFlag(result, Common::RemoveFileFlags::FlagFileRemoved);
                Common::ApplyFlag(result, item.getAddedAsDirectoryFlag(), Common::RemoveFileFlags::FlagFullRepository);

                Q_ASSERT(item.m_FilesCount >= 0);
                if (item.m_FilesCount == 0) {
                    item.setIsRemovedFlag(true);
                    Common::SetFlag(result, Common::RemoveFileFlags::FlagRepositoryEmpty);
                }

                m_FilesSet.remove(filepath);
            } else {
                Q_ASSERT(false);
            }
        }

        return result;
    }

    void ArtworksRepository::removeVector(const QString &vectorPath) {
        m_FilesWatcher.removePath(vectorPath);
    }

    void ArtworksRepository::cleanupEmptyDirectories() {
        LOG_DEBUG << "#";
        const size_t size = m_DirectoriesList.size();
        std::vector<int> indicesToRemove;
        indicesToRemove.reserve(size);

        for (size_t i = 0; i < size; ++i) {
            auto &directory = m_DirectoriesList[i];
            if (directory.m_FilesCount == 0) {
                indicesToRemove.push_back((int)i);
            }
        }

        if (!indicesToRemove.empty()) {
            LOG_INFO << indicesToRemove.size() << "empty directory(ies)...";
            removeItems(Helpers::IndicesRanges(indicesToRemove));
            updateSelectedState();
        }
    }

    void ArtworksRepository::purgeUnavailableFiles() {
        LOG_DEBUG << "#";
        m_UnavailableFiles.clear();
        m_LastUnavailableFilesCount = 0;
    }

    void ArtworksRepository::watchFiles(const Artworks::ArtworksSnapshot &snapshot) {
        LOG_DEBUG << snapshot.size() << "item(s)";
        if (snapshot.empty()) { return; }

        QStringList filepaths;
        filepaths.reserve((int)snapshot.size());
        for (auto &artwork: snapshot) {
            filepaths.append(artwork->getFilepath());

            auto imageArtwork = std::dynamic_pointer_cast<Artworks::ImageArtwork>(artwork);
            if ((imageArtwork != nullptr) && imageArtwork->hasVectorAttached()) {
                filepaths.append(imageArtwork->getAttachedVectorPath());
            }
        }

        m_FilesWatcher.addPaths(filepaths);

        emit dataChanged(this->index(0), this->index(rowCount() - 1));
        emit refreshRequired();
    }

    void ArtworksRepository::setFullDirectories(const QSet<qint64> &directoryIDs) {
        LOG_DEBUG << directoryIDs.size() << "item(s)";
        for (auto id: directoryIDs) {
            size_t index = 0;
            if (tryFindDirectoryByID(id, index)) {
                m_DirectoriesList[index].setAddedAsDirectoryFlag(true);
            }
        }
    }

    void ArtworksRepository::removeFiles(const Artworks::ArtworksSnapshot &snapshot, ArtworksRemoveResult &removeResult) {
        if (snapshot.empty()) { return; }
        QStringList filepaths;
        QStringList removedAttachedVectors;
        filepaths.reserve((int)snapshot.size());
        removedAttachedVectors.reserve((int)snapshot.size()/2);

        for (auto &artwork: snapshot) {
            auto flags = removeFile(artwork->getFilepath(), artwork->getDirectoryID());
            if (!Common::HasFlag(flags, Common::RemoveFileFlags::FlagFileRemoved)) { continue; }

            filepaths.append(artwork->getFilepath());

            auto image = std::dynamic_pointer_cast<Artworks::ImageArtwork>(artwork);
            if (image != NULL && image->hasVectorAttached()) {
                removedAttachedVectors.append(image->getAttachedVectorPath());
            }

            if (Common::HasFlag(flags, Common::RemoveFileFlags::FlagRepositoryEmpty) &&
                    Common::HasFlag(flags, Common::RemoveFileFlags::FlagFullRepository)) {
                removeResult.m_FullDirectoryIds.insert(artwork->getDirectoryID());
            }
        }

        unwatchFilePaths(filepaths);
        unwatchFilePaths(removedAttachedVectors);

        const size_t beforeSelectedCount = retrieveSelectedDirsCount();
        removeResult.m_SelectedDirectoryIds = consolidateSelectionForEmpty();
        const size_t afterSelectedCount = retrieveSelectedDirsCount();
        // current selection logic: if all directories become deselected after some action, all become selected
        // this if statement is supposed to check if this has happened
        removeResult.m_UnselectAll = (afterSelectedCount + removeResult.m_SelectedDirectoryIds.size()) != beforeSelectedCount;

        emit dataChanged(this->index(0), this->index(rowCount() - 1));
        emit refreshRequired();
    }

    void ArtworksRepository::unwatchFilePaths(const QStringList &filePaths) {
        if (!filePaths.empty()) {
            m_FilesWatcher.removePaths(filePaths);
        }
    }

    void ArtworksRepository::updateFilesCounts() {
        if (m_DirectoriesList.empty()) { return; }

        auto first = this->index(0);
        auto last = this->index(rowCount() - 1);
        emit dataChanged(first, last, QVector<int>() << UsedImagesCountRole);

        emit refreshRequired();
    }

    void ArtworksRepository::updateSelectedState() {
        if (m_DirectoriesList.empty()) { return; }

        auto first = this->index(0);
        auto last = this->index(rowCount() - 1);
        emit dataChanged(first, last, QVector<int>() << IsSelectedRole);
    }

    void ArtworksRepository::watchFilePath(const QString &filepath) {
#ifndef CORE_TESTS
        m_FilesWatcher.addPath(filepath);
#else
        Q_UNUSED(filepath);
#endif
    }

    bool ArtworksRepository::tryGetDirectoryPath(qint64 directoryID, QString &absolutePath) const {
        bool found = false;

        size_t index;
        if (tryFindDirectoryByID(directoryID, index)) {
            absolutePath = m_DirectoriesList[index].m_AbsolutePath;
            found = true;
        }

        return found;
    }

    bool ArtworksRepository::isFileUnavailable(const QString &filepath) const {
        bool isUnavailable = false;

        if (m_UnavailableFiles.contains(filepath)) {
#if defined(CORE_TESTS) || defined(INTEGRATION_TESTS)
            {
#else
            if(!QFileInfo(filepath).exists()) {
#endif

                isUnavailable = true;
            }
        }

        return isUnavailable;
    }

#if defined(INTEGRATION_TESTS) || defined(UI_TESTS)
    void ArtworksRepository::resetEverything() {
        m_DirectoriesList.clear();
        m_FilesSet.clear();
    }
#endif

    QStringList ArtworksRepository::retrieveFullDirectories() const {
        LOG_DEBUG << "#";
        QStringList directoriesList;

        for (auto &dir: m_DirectoriesList) {
            if (dir.getAddedAsDirectoryFlag() &&
                    !dir.getIsRemovedFlag()) {
                directoriesList.push_back(dir.m_AbsolutePath);
            }
        }

        LOG_VERBOSE_OR_DEBUG << directoriesList;

        return directoriesList;
    }

    void ArtworksRepository::restoreFullDirectories(const QStringList &directories) {
        LOG_DEBUG << directories.size() << "directory(ies)";

        bool anyChanged = false;

        for (auto &dir: directories) {
            size_t index = 0;
            if (tryFindDirectory(dir, index)) {
                LOG_DEBUG << dir << "marked as full";
                m_DirectoriesList[index].setAddedAsDirectoryFlag(true);
                anyChanged = true;
            } else {
                LOG_WARNING << "Failed to find directory" << dir;
            }
        }

#ifdef QT_DEBUG
        if (anyChanged) {
            emit dataChanged(this->index(0), this->index(this->rowCount() - 1), QVector<int>() << IsFullRole);
        }
#endif
    }

    int ArtworksRepository::rowCount(const QModelIndex &parent) const {
        Q_UNUSED(parent);
        return (int)m_DirectoriesList.size();
    }

    QVariant ArtworksRepository::data(const QModelIndex &index, int role) const {
        int row = index.row();
        if ((row < 0) || (row >= (int)m_DirectoriesList.size())) {
            return QVariant();
        }

        auto &directory = m_DirectoriesList.at(index.row());

        switch (role) {
            case PathRole: {
                QDir dir(directory.m_AbsolutePath);
                return dir.dirName();
            }
            case UsedImagesCountRole:
                return directory.m_FilesCount;
            case IsSelectedRole:
                return !allAreSelected() && directory.getIsSelectedFlag();
            case IsRemovedRole:
                return directory.getIsRemovedFlag();
            case IsFullRole:
                return directory.getAddedAsDirectoryFlag();
            default:
                return QVariant();
        }
    }

    QSet<qint64> ArtworksRepository::consolidateSelectionForEmpty() {
        LOG_DEBUG << "#";
        QSet<qint64> result;

        bool anyChange = false;

        if (!allAreSelected()) {
            const bool newIsSelected = false; // unselect folder to be deleted
            const size_t size = m_DirectoriesList.size();

            for (size_t i = 0; i < size; i++) {
                auto &directory = m_DirectoriesList[i];
                if (!directory.isValid()) {
                    const bool oldIsSelected = directory.getIsSelectedFlag();
                    if (oldIsSelected) {
                        result.insert(directory.m_ID);
                    }

                    if (changeSelectedState(i, newIsSelected, oldIsSelected)) {
                        anyChange = true;
                    }
                }
            }
        }

        if (anyChange) {
            emit selectionChanged();
        }

        emit artworksSourcesChanged();
        return result;
    }

    void ArtworksRepository::restoreDirectoriesSelection(const QSet<qint64> &selectedDirectories) {
        LOG_DEBUG << "#";
        if (selectedDirectories.empty()) { return; }

        const size_t size = m_DirectoriesList.size();

        for (size_t i = 0; i < size; i++) {
            auto &directory = m_DirectoriesList[i];
            if (selectedDirectories.contains(directory.m_ID)) {
                directory.setIsSelectedFlag(true);
            }
        }
    }

    void ArtworksRepository::toggleDirectorySelected(size_t row) {
        LOG_INFO << row;
        if (row >= m_DirectoriesList.size()) { return; }
        Q_ASSERT(m_DirectoriesList[row].isValid());

        auto &directory = m_DirectoriesList.at(row);

        const bool oldValue = directory.getIsSelectedFlag();
        const bool newValue = !oldValue;
        LOG_DEBUG << row << "old" << oldValue << "new" << newValue;

        if (changeSelectedState(row, newValue, oldValue)) {
            updateSelectedState();
            emit selectionChanged();
        }
    }

    bool ArtworksRepository::setDirectorySelected(size_t index, bool value) {
        auto &directory = m_DirectoriesList[index];
        //if (!directory.isValid()) { return false; }

        bool changed = directory.getIsSelectedFlag() != value;
        directory.setIsSelectedFlag(value);

        return changed;
    }

    bool ArtworksRepository::changeSelectedState(size_t row, bool newValue, bool oldValue) {
        Q_ASSERT(row < m_DirectoriesList.size());
        if (oldValue == newValue) { return false; }

        bool anySelectionChanged = false;

        const size_t count = m_DirectoriesList.size();
        if (count == 1) {
            Q_ASSERT(row == 0);
            anySelectionChanged = setDirectorySelected(row, true);
            return anySelectionChanged;
        }

        const size_t selectedCount = retrieveSelectedDirsCount();
        const bool allAreSelected = selectedCount == count;
        const bool wasSelected = (oldValue == true);
        const bool onlyOneSelected = (selectedCount == 1);
        const bool isUnselected = (newValue == false);

        if (allAreSelected && (count > 1)) {
            Q_ASSERT(newValue == false);
            unselectAllDirectories();
            setDirectorySelected(row, true);
            anySelectionChanged = true;
        } else if (onlyOneSelected && wasSelected && isUnselected) {
            selectAllDirectories();
            anySelectionChanged = true;
        } else {
            anySelectionChanged = setDirectorySelected(row, newValue);
        }

        return anySelectionChanged;
    }

    bool ArtworksRepository::setAllSelected(bool value) {
        bool anyChange = false;
        size_t size = m_DirectoriesList.size();

        for (size_t i = 0; i < size; i++) {
            if (setDirectorySelected(i, value)) {
                anyChange = true;
            }
        }

        if (anyChange) {
            updateSelectedState();
        }

        return anyChange;
    }

    size_t ArtworksRepository::retrieveSelectedDirsCount() const {
        size_t count = 0;

        for (auto &directory: m_DirectoriesList) {
            if (directory.getIsSelectedFlag()) {
                count++;
            }
        }

        return count;
    }

    bool ArtworksRepository::allAreSelected() const {
        bool anyUnselected = false;

        for (auto &item: m_DirectoriesList) {
            if (!item.getIsSelectedFlag()) {
                anyUnselected = true;
                break;
            }
        }

        return !anyUnselected;
    }

    bool ArtworksRepository::tryFindDirectory(const QString &directoryPath, size_t &index) const {
        bool found = false;
        const size_t size = m_DirectoriesList.size();

        for (size_t i = 0; i < size; ++i) {
            auto &item = m_DirectoriesList[i];
            if (QString::compare(item.m_AbsolutePath, directoryPath, Qt::CaseInsensitive) == 0) {
                found = true;
                index = i;
                break;
            }
        }

        return found;
    }

    bool ArtworksRepository::tryFindDirectoryByID(qint64 id, size_t &index) const {
        bool found = false;
        const size_t size = m_DirectoriesList.size();

        for (size_t i = 0; i < size; ++i) {
            auto &item = m_DirectoriesList[i];
            if (item.m_ID == id) {
                found = true;
                index = i;
                break;
            }
        }

        return found;
    }

    QHash<int, QByteArray> ArtworksRepository::roleNames() const {
        QHash<int, QByteArray> roles;
        roles[PathRole] = "path";
        roles[UsedImagesCountRole] = "usedimagescount";
        roles[IsSelectedRole] = "isselected";
        roles[IsRemovedRole] = "isremoved";
        roles[IsFullRole] = "isfull";
        return roles;
    }

    void ArtworksRepository::removeInnerItem(int index) {
        auto &directoryToRemove = m_DirectoriesList.at(index);
        if (!allAreSelected()) {
            const bool oldIsSelected = directoryToRemove.getIsSelectedFlag();
            const bool newIsSelected = false; // unselect folder to be deleted
            changeSelectedState(index, newIsSelected, oldIsSelected);
        }
        m_DirectoriesList.erase(m_DirectoriesList.begin() + index);
    }

    /*virtual */
    bool ArtworksRepository::checkFileExists(const QString &filename, QString &directory) const {
        bool exists = false;
        QFileInfo fi(filename);

#if !defined(CORE_TESTS) && !defined(UI_TESTS)
        exists = fi.exists();
#else
        exists = true;
#endif

        if (exists) {
            directory = fi.absolutePath();
        }

        return exists;
    }

    void ArtworksRepository::checkFileUnavailable(const QString &path) {
        LOG_INFO << "File changed:" << path;

        QFileInfo fi(path);
        if (!fi.exists()) {
            LOG_INFO << "File become unavailable:" << path;
            m_UnavailableFiles.insert(fi.absoluteFilePath());
            LOG_DEBUG << "Starting availability timer...";
            m_Timer.start();
        }
    }

    void ArtworksRepository::onAvailabilityTimer() {
        int currentUnavailableSize = m_UnavailableFiles.size();
        LOG_INFO << "Current:" << currentUnavailableSize << "Last:" << m_LastUnavailableFilesCount;

        if (currentUnavailableSize > m_LastUnavailableFilesCount) {
            m_LastUnavailableFilesCount = currentUnavailableSize;
            emit filesUnavailable();
        }
    }

    FilteredArtworksRepository::FilteredArtworksRepository(ArtworksRepository &artworksRepository):
        m_ArtworksRepository(artworksRepository)
    {
        setArtworksRepository(m_ArtworksRepository);
    }

    void FilteredArtworksRepository::setArtworksRepository(ArtworksRepository &artworksRepository) {
        setSourceModel(&artworksRepository);

        QObject::connect(&artworksRepository, &ArtworksRepository::artworksSourcesChanged,
                         this, &FilteredArtworksRepository::artworksSourcesCountChanged);
        QObject::connect(&artworksRepository, &ArtworksRepository::refreshRequired,
                         this, &FilteredArtworksRepository::onRefreshRequired);
    }

    int FilteredArtworksRepository::getOriginalIndex(int index) {
        LOG_INFO << index;
        QModelIndex originalIndex = mapToSource(this->index(index, 0));
        int row = originalIndex.row();
        return row;
    }

    void FilteredArtworksRepository::selectDirectory(int row) {
        LOG_INFO << row;
        int originalRow = getOriginalIndex(row);
        m_ArtworksRepository.toggleDirectorySelected(originalRow);
        emit directoriesFiltered();
    }

    bool FilteredArtworksRepository::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const {
        Q_UNUSED(sourceParent);

#if !defined(CORE_TESTS) && !defined(INTEGRATION_TESTS) && defined(QT_DEBUG)
        Q_UNUSED(sourceRow);
        return true;
#else
        bool isEmpty = m_ArtworksRepository.isEmpty(sourceRow);
        return !isEmpty;
#endif
    }

    void FilteredArtworksRepository::onRefreshRequired() {
        LOG_DEBUG << "#";
        this->invalidate();
    }
}

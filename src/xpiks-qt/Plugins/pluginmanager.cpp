/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "pluginmanager.h"

#include <cstddef>

#include <Common/logging.h>
#include <QAbstractItemModel>
#include <QByteArray>
#include <QChar>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QPluginLoader>
#include <QQmlEngine>
#include <QStringList>
#include <QUrl>
#include <QVector>
#include <QtDebug>
#include <QtGlobal>

#include "Common/flags.h"
#include "Common/isystemenvironment.h"
#include "Helpers/constants.h"
#include "Helpers/filehelpers.h"
#include "Plugins/pluginactionsmodel.h"
#include "Plugins/pluginwrapper.h"
#include "Plugins/sandboxeddependencies.h"
#include "Plugins/uiprovider.h"
#include "Plugins/xpiksplugininterface.h"

namespace Microstocks {
    class IMicrostockAPIClients;
}

namespace Models {
    class UIManager;
}

namespace Storage {
    class DatabaseManager;
}

namespace Plugins {
    PluginManager::PluginManager(Common::ISystemEnvironment &environment,
                                 Commands::ICommandManager &commandManager,
                                 KeywordsPresets::IPresetsManager &presetsManager,
                                 Storage::DatabaseManager &dbManager,
                                 Connectivity::RequestsService &requestsService,
                                 Microstocks::IMicrostockAPIClients &microstockClients,
                                 Models::ICurrentEditableSource &currentEditableSource,
                                 Models::UIManager &uiManager):
        QAbstractListModel(),
        m_Environment(environment),
        m_CommandManager(commandManager),
        m_PresetsManager(presetsManager),
        m_DatabaseManager(dbManager),
        m_MicrostockServices(microstockClients, requestsService),
        m_CurrentEditableSource(currentEditableSource),
        m_UIProvider(uiManager),
        m_LastPluginID(0)
    { }

    PluginManager::~PluginManager() {
        LOG_DEBUG << "#";
    }

    bool PluginManager::initialize() {
        LOG_DEBUG << "#";
        bool success = m_Environment.ensureDirExists(Constants::PLUGINS_DIR);
        if (!success) { return false; }

        m_PluginsDirectoryPath = m_Environment.path({Constants::PLUGINS_DIR});

        m_FailedPluginsDirectory = m_Environment.path({Constants::PLUGINS_DIR, Constants::FAILED_PLUGINS_DIR});
        Helpers::ensureDirectoryExists(m_FailedPluginsDirectory);

        LOG_INFO << "Plugins directory:" << m_PluginsDirectoryPath;
        return true;
    }

    void PluginManager::processInvalidFile(const QString &filename, const QString &pluginFullPath) {
        LOG_DEBUG << pluginFullPath;

        const QString failedDestination = QDir::cleanPath(m_FailedPluginsDirectory + QChar('/') + filename);
        if (QFile::rename(pluginFullPath, failedDestination)) {
            LOG_INFO << "Moved invalid file from the plugins dir" << pluginFullPath;
        } else {
            LOG_WARNING << "Failed to move invalid file from plugins dir. Attempting remove...";
            if (QFile::remove(pluginFullPath)) {
                LOG_INFO << "Removed invalid file from plugins dir";
            } else {
                LOG_WARNING << "Failed to remove invalid file from plugins dir";
            }
        }
    }

    void PluginManager::loadPlugins() {
        LOG_DEBUG << "#";

        if (!initialize()) {
            LOG_WARNING << "Failed to initialize plugins directory. Exiting...";
            return;
        }

        QDir pluginsDir(getPluginsDirectoryPath());
        Q_ASSERT(pluginsDir.exists());

        std::vector<std::shared_ptr<PluginWrapper> > loadedPlugins;
        QHash<int, std::shared_ptr<PluginWrapper> > pluginsDict;

        QStringList pluginDirFiles = pluginsDir.entryList(QDir::Files);
        LOG_INFO << "Trying to load" << pluginDirFiles.size() << "file(s)";

        foreach (QString fileName, pluginDirFiles) {
            LOG_DEBUG << "Trying file:" << fileName;

            const QString pluginFullPath = pluginsDir.absoluteFilePath(fileName);
            auto pluginWrapper = loadPlugin(pluginFullPath);
            if (pluginWrapper) {
                loadedPlugins.push_back(pluginWrapper);
                Q_ASSERT(!pluginsDict.contains(pluginWrapper->getPluginID()));
                pluginsDict.insert(pluginWrapper->getPluginID(), pluginWrapper);
            } else {
                processInvalidFile(fileName, pluginFullPath);
            }
        }

        if (!loadedPlugins.empty()) {
            LOG_DEBUG << "Resetting plugins model";
            beginResetModel();
            {
                m_PluginsList.swap(loadedPlugins);
                m_PluginsDict.swap(pluginsDict);

                loadedPlugins.clear();
                pluginsDict.clear();
            }
            endResetModel();
        }
    }

    void PluginManager::unloadPlugins() {
        m_UIProvider.closeAllDialogs();

        const size_t size = m_PluginsList.size();
        LOG_DEBUG << size << "plugin(s)";

        for (size_t i = 0; i < size; ++i) {
            auto &wrapper = m_PluginsList.at(i);

            if (wrapper->getIsRemoved()) {
                LOG_INFO << "Plugin" << wrapper->getPluginID() << "is already removed";
                continue;
            }

            wrapper->disablePlugin();
            wrapper->finalizePlugin();
        }

        m_PluginsList.clear();
        m_PluginsDict.clear();

        // plugins should be automagically unloaded
    }

    bool PluginManager::hasExportedActions(int row) const {
        bool hasActions = false;

        if ((0 <= row) && (row < rowCount())) {
            hasActions = m_PluginsList.at(row)->anyActionsProvided();
        }

        return hasActions;
    }

    bool PluginManager::isUsable(int row) const {
        bool isUsable = false;

        if ((0 <= row) && (row < rowCount())) {
            auto &wrapper = m_PluginsList.at(row);
            isUsable = wrapper->getIsEnabled() && !wrapper->getIsRemoved();
        }

        return isUsable;
    }

    void PluginManager::onCurrentEditableChanged() {
        LOG_DEBUG << "#";
        size_t size = m_PluginsList.size();
        QVariant empty;

        for (size_t i = 0; i < size; ++i) {
            std::shared_ptr<PluginWrapper> &wrapper = m_PluginsList.at(i);
            wrapper->notifyPlugin(Common::PluginNotificationFlags::CurrentEditableChanged, empty, nullptr);
        }
    }

    void PluginManager::onPresetsUpdated() {
        LOG_DEBUG << "#";
        size_t size = m_PluginsList.size();
        QVariant empty;

        for (size_t i = 0; i < size; ++i) {
            std::shared_ptr<PluginWrapper> &wrapper = m_PluginsList.at(i);
            wrapper->notifyPlugin(Common::PluginNotificationFlags::PresetsUpdated, empty, nullptr);
        }
    }

    QObject *PluginManager::getPluginActions(int index) const {
        LOG_DEBUG << index;
        PluginActionsModel *item = NULL;

        if ((0 <= index) && (index < rowCount())) {
            item = m_PluginsList.at(index)->getActionsModel();
            QQmlEngine::setObjectOwnership(item, QQmlEngine::CppOwnership);
        }

        return item;
    }

    void PluginManager::triggerPluginAction(int pluginID, int actionID) const {
        LOG_INFO << "Plugin ID" << pluginID << "action ID" << actionID;
        std::shared_ptr<PluginWrapper> pluginWrapper = m_PluginsDict.value(pluginID, std::shared_ptr<PluginWrapper>());
        if (pluginWrapper) {
            pluginWrapper->triggerActionSafe(actionID);
        }
    }

    bool PluginManager::removePlugin(int index) {
        LOG_INFO << "index:" << index;
        bool removed = false;

        QPluginLoader loader;

        if ((0 <= index) && (index < rowCount())) {
            std::shared_ptr<PluginWrapper> wrapper = m_PluginsList.at(index);
            const int pluginID = wrapper->getPluginID();
            LOG_INFO << "Removing plugin with ID:" << pluginID;

            wrapper->disablePlugin();
            wrapper->finalizePlugin();

            loader.setFileName(wrapper->getFilepath());

            wrapper->removePlugin();
            emit dataChanged(this->index(index), this->index(index), QVector<int>() << IsRemovedRole);
            /*beginRemoveRows(QModelIndex(), index, index);
            {
                m_PluginsList.erase(m_PluginsList.begin() + index);
            }
            endRemoveRows();*/

            int removedCount = m_PluginsDict.remove(pluginID);
            Q_ASSERT(removedCount == 1);

            const QString fullPath = wrapper->getFilepath();
            LOG_DEBUG << "Attempting to remove file" << fullPath;
            if (!QFile::remove(fullPath)) {
                LOG_WARNING << "Failed to remove file" << fullPath;
            }

            LOG_INFO << "Plugin with ID #" << pluginID << "removed";
            removed = true;
        }

        if (removed) {
            Q_ASSERT(loader.isLoaded());
            bool canUnload = loader.unload();
            LOG_INFO << "Can unload the plugin:" << canUnload;
        }

        return removed;
    }

    bool PluginManager::pluginExists(const QUrl &pluginUrl) {
        bool exists = false;
        QString fullpath = pluginUrl.toLocalFile();

        QFileInfo existingFI(fullpath);
        if (existingFI.exists()) {
            const QString filename = existingFI.fileName();
            QString destinationPath = QDir::cleanPath(m_PluginsDirectoryPath + QChar('/') + filename);
            exists = QFileInfo(destinationPath).exists() || isPluginAdded(destinationPath);
        }

        return exists;
    }

    bool PluginManager::installPlugin(const QUrl &pluginUrl) {
        bool result = addPlugin(pluginUrl.toLocalFile());
        return result;
    }

    bool PluginManager::addPlugin(const QString &fullpath) {
        LOG_INFO << fullpath;
        bool success = false;

        do {
            QFileInfo existingFI(fullpath);
            if (!existingFI.exists()) {
                LOG_WARNING << "Path not found:" << fullpath;
                break;
            }

            const QString filename = existingFI.fileName();
            QString destinationPath = QDir::cleanPath(m_PluginsDirectoryPath + QChar('/') + filename);
            if (QFileInfo(destinationPath).exists() || isPluginAdded(destinationPath)) {
                LOG_WARNING << "Plugin with same filename already added";
                break;
            }

            LOG_DEBUG << "Copying [" << fullpath << "] to [" << destinationPath << "]";
            if (!QFile::copy(fullpath, destinationPath)) {
                LOG_WARNING << "Failed to copy plugin to" << destinationPath;
                break;
            }

            if (!doAddPlugin(destinationPath)) {
                if (!QFile::remove(destinationPath)) {
                    LOG_WARNING << "Failed to remove file" << destinationPath;
                }

                break;
            }

            LOG_INFO << "Added plugin" << fullpath;
            success = true;
        } while (false);

        return success;
    }

    bool PluginManager::doAddPlugin(const QString &filepath) {
        bool added = false;
        auto plugin = loadPlugin(filepath);
        if (plugin) {
            const int size = (int)m_PluginsList.size();

            beginInsertRows(QModelIndex(), size, size);
            {
                m_PluginsList.push_back(plugin);
            }
            endInsertRows();

            const int pluginID = plugin->getPluginID();
            m_PluginsDict.insert(pluginID, plugin);

            added = true;
        }

        return added;
    }

    int PluginManager::findPluginIndex(const QString &fullpath) const {
        LOG_INFO << fullpath;
        int index = -1;

        const size_t size = m_PluginsList.size();
        for (size_t i = 0; i < size; i++) {
            const auto &wrapper = m_PluginsList.at(i);
            if (wrapper->getFilepath() == fullpath) {
                index = (int)i;
                break;
            }
        }

        return index;
    }

    std::shared_ptr<PluginWrapper> PluginManager::loadPlugin(const QString &filepath) {
        LOG_INFO << filepath;
        std::shared_ptr<PluginWrapper> result;

        try {
            bool success = false;
            QPluginLoader loader(filepath);
            do {
                QObject *plugin = loader.instance();
                if (!plugin) {
                    LOG_WARNING << loader.errorString();
                    break;
                }

                XpiksPluginInterface *xpiksPlugin = qobject_cast<XpiksPluginInterface *>(plugin);
                if (!xpiksPlugin) {
                    LOG_DEBUG << "Not Xpiks Plugin:" << filepath;
                    break;
                }

                result = instantiatePlugin(filepath, xpiksPlugin);
                if (!result) {
                    break;
                }

                success = true;
            } while (false);

            if (!success) {
                loader.unload();
            }
        } catch(...) {
            LOG_WARNING << "Exception while loading" << filepath;
        }

        return result;
    }

    std::shared_ptr<PluginWrapper> PluginManager::instantiatePlugin(const QString &filepath, XpiksPluginInterface *plugin) {
        const int pluginID = getNextPluginID();
        LOG_INFO << "ID:" << pluginID << "name:" << plugin->getPrettyName() << "version:" << plugin->getVersionString() << "filepath:" << filepath;

        auto pluginWrapper = std::make_shared<PluginWrapper>(filepath,
                                                             plugin,
                                                             pluginID,
                                                             m_Environment,
                                                             m_UIProvider,
                                                             m_DatabaseManager);
        bool initialized = false;

        do {
            try {
                plugin->injectCommandManager(&m_CommandManager);
                plugin->injectUIProvider(&pluginWrapper->getUIProvider());
                plugin->injectPresetsManager(&m_PresetsManager);
                plugin->injectDatabaseManager(pluginWrapper->getDatabaseManager());
                plugin->injectMicrostockServices(&m_MicrostockServices);
                plugin->injectCurrentEditable(&m_CurrentEditableSource);
            }
            catch(...) {
                LOG_WARNING << "Failed to inject dependencies to plugin with ID:" << pluginID;
                break;
            }

            if (!pluginWrapper->initializePlugin()) { break; }

            // TODO: check this in config in future
            if (!pluginWrapper->enablePlugin()) { break; }

            initialized = true;
        } while(false);

        if (!initialized) {
            pluginWrapper->finalizePlugin();

            LOG_WARNING << "Fail initializing plugin with ID:" << pluginID;
            pluginWrapper.reset();
        }

        return pluginWrapper;
    }

    int PluginManager::rowCount(const QModelIndex &parent) const {
        Q_UNUSED(parent);
        return (int)m_PluginsList.size();
    }

    QVariant PluginManager::data(const QModelIndex &index, int role) const {
        int row = index.row();
        if (row < 0 || row >= rowCount()) { return QVariant(); }

        auto &plugin = m_PluginsList.at(row);

        switch (role) {
        case PrettyNameRole:
            return plugin->getPrettyName();
        case AuthorRole:
            return plugin->getAuthor();
        case VersionRole:
            return plugin->getVersionString();
        case PluginIDRole:
            return plugin->getPluginID();
        case IsEnabledRole:
            return plugin->getIsEnabled();
        case IsRemovedRole:
            return plugin->getIsRemoved();
        default:
            return QVariant();
        }
    }

    QHash<int, QByteArray> PluginManager::roleNames() const {
        QHash<int, QByteArray> roles;
        roles[PrettyNameRole] = "prettyname";
        roles[AuthorRole] = "author";
        roles[VersionRole] = "version";
        roles[PluginIDRole] = "pluginID";
        roles[IsEnabledRole] = "enabled";
        roles[IsRemovedRole] = "removed";
        return roles;
    }

    PluginsWithActionsModel::PluginsWithActionsModel(PluginManager &pluginManager, QObject *parent):
        QSortFilterProxyModel(parent)
    {
        setSourceModel(&pluginManager);
    }

    int PluginsWithActionsModel::getOriginalIndex(int index) {
        QModelIndex originalIndex = mapToSource(this->index(index, 0));
        int row = originalIndex.row();

        return row;
    }

    bool PluginsWithActionsModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const {
        Q_UNUSED(sourceParent);

        QAbstractItemModel *sourceItemModel = sourceModel();
        PluginManager *pluginManager = dynamic_cast<PluginManager *>(sourceItemModel);
        Q_ASSERT(pluginManager != NULL);
        bool result = pluginManager->isUsable(sourceRow) && pluginManager->hasExportedActions(sourceRow);
        return result;
    }
}

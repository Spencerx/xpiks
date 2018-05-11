/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "xpiksapp.h"
#include <apisecretsstorage.h>
#include <QQmlContext>
#include <QUuid>
#include "Encryption/aes-qt.h"

XpiksApp::XpiksApp(Common::ISystemEnvironment &environment):
    m_LogsModel(&m_ColorsModel),
    m_SettingsModel(environment),
    m_FilteredArtworksRepository(&m_ArtworksRepository),
    m_UploadInfoRepository(environment),
    m_RequestsService(m_SettingsModel.getProxySettings()),
    m_PresetsModel(environment),
    m_WarningsService(environment),
    m_DatabaseManager(environment),
    m_SecretsStorage(new libxpks::microstocks::APISecretsStorage()),
    m_ApiClients(m_SecretsStorage.get()),
    m_KeywordsSuggestor(m_ApiClients, m_RequestsService, environment),
    m_RecentDirectorieModel(environment),
    m_RecentFileModel(environment),
    m_FtpCoordinator(new libxpks::net::FtpCoordinator(m_SettingsModel.getMaxParallelUploads())),
    m_ArtworkUploader(environment, m_FtpCoordinator, m_UploadInfoRepository),
    m_SpellCheckerService(environment, &m_SettingsModel),
    m_MetadataIOService(&m_DatabaseManager),
    m_AutoCompleteService(&m_AutoCompleteModel, &m_PresetsModel, &m_SettingsModel),
    m_ImageCachingService(environment, &m_DatabaseManager),
    m_ReplaceModel(&m_ColorsModel),
    m_TranslationManager(environment),
    m_TranslationService(m_TranslationManager),
    m_UIManager(environment, &m_SettingsModel),
    m_SessionManager(environment),
    m_MaintenanceService(environment),
    m_VideoCachingService(environment, &m_DatabaseManager),
    m_SwitcherModel(environment),
    m_DuplicatesModel(&m_ColorsModel),
    m_CsvExportModel(environment),
    m_UpdateService(environment, &m_SettingsModel, &m_SwitcherModel, &m_MaintenanceService),
    m_PluginManager(environment, &m_DatabaseManager, m_RequestsService, m_ApiClients),
    m_HelpersQmlWrapper(environment, &m_ColorsModel)
{
    m_FilteredPresetsModel.setSourceModel(&m_PresetsModel);
    m_FilteredArtItemsModel.setSourceModel(&m_ArtItemsModel);

    m_WarningsModel.setSourceModel(&m_ArtItemsModel);
    m_WarningsModel.setWarningsSettingsModel(m_WarningsService.getWarningsSettingsModel());

    m_ArtworksUpdateHub.setStandardRoles(m_ArtItemsModel.getArtworkStandardRoles());

    m_PluginsWithActions.setSourceModel(&m_PluginManager);

    QObject::connect(&m_InitCoordinator, &Helpers::AsyncCoordinator::statusReported,
                     this, &XpiksApp::servicesInitialized);
}

XpiksApp::~XpiksApp() {
    m_LogsModel.stopLogging();
}

bool XpiksApp::getIsUpdateDownloaded() {
    return m_UpdateService.getIsUpdateDownloaded();
}

bool XpiksApp::getPluginsAvailable() const {
    bool result = false;
#ifdef WITH_PLUGINS
    result = true;
#endif
    return result;
}

Plugins::UIProvider *XpiksApp::getUIProvider() {
    return m_PluginManager.getUIProvider();
}

void XpiksApp::startLogging() {
    m_LogsModel.startLogging();
}

void XpiksApp::initialize() {
    m_SettingsModel.initializeConfigs();
    m_SettingsModel.retrieveAllValues();

#ifndef INTEGRATION_TESTS
    ensureUserIdExists();

    QString userId = m_SettingsModel.getUserAgentId();
    userId.remove(QRegExp("[{}-]."));

#if defined(QT_NO_DEBUG)
    const bool telemetryEnabled = settingsModel.getIsTelemetryEnabled();
#else
    const bool telemetryEnabled = false;
#endif

    m_TelemetryService.initialize(userId, telemetryEnabled);
#else
    m_TelemetryService.initialize("1234567890", false);
#endif

    injectDependencies();

    // other initializations
    m_SecretsManager.setMasterPasswordHash(m_SettingsModel.getMasterPasswordHash());
    m_RecentDirectorieModel.initialize();
    m_RecentFileModel.initialize();

    connectEntitiesSignalsSlots();

    m_LanguagesModel.initFirstLanguage();
    m_LanguagesModel.loadLanguages();

    m_ColorsModel.initializeBuiltInThemes();
    m_LogsModel.InjectDependency(&m_ColorsModel);

    m_TelemetryService.setInterfaceLanguage(m_LanguagesModel.getCurrentLanguage());
    m_ColorsModel.applyTheme(m_SettingsModel.getSelectedThemeIndex());

    m_PluginManager.getUIProvider()->setUIManager(&m_UIManager);
}

void XpiksApp::setupUI(QQmlContext *context) {
    context->setContextProperty("artItemsModel", &m_ArtItemsModel);
    context->setContextProperty("artworkRepository", &m_FilteredArtworksRepository);
    context->setContextProperty("combinedArtworks", &m_CombinedArtworksModel);
    context->setContextProperty("secretsManager", &m_SecretsManager);
    context->setContextProperty("undoRedoManager", &m_UndoRedoManager);
    context->setContextProperty("keywordsSuggestor", &m_KeywordsSuggestor);
    context->setContextProperty("settingsModel", &m_SettingsModel);
    context->setContextProperty("filteredArtItemsModel", &m_FilteredArtItemsModel);
    context->setContextProperty("helpersWrapper", &m_HelpersQmlWrapper);
    context->setContextProperty("recentDirectories", &m_RecentDirectorieModel);
    context->setContextProperty("recentFiles", &m_RecentFileModel);
    context->setContextProperty("metadataIOCoordinator", &m_MetadataIOCoordinator);
    context->setContextProperty("pluginManager", &m_PluginManager);
    context->setContextProperty("pluginsWithActions", &m_PluginsWithActions);
    context->setContextProperty("warningsModel", &m_WarningsModel);
    context->setContextProperty("languagesModel", &m_LanguagesModel);
    context->setContextProperty("i18", &m_LanguagesModel);
    context->setContextProperty("uiColors", &m_ColorsModel);
    context->setContextProperty("acSource", &m_AutoCompleteModel);
    context->setContextProperty("replaceModel", &m_ReplaceModel);
    context->setContextProperty("presetsModel", &m_PresetsModel);
    context->setContextProperty("filteredPresetsModel", &m_FilteredPresetsModel);
    context->setContextProperty("artworkProxy", &m_ArtworkProxyModel);
    context->setContextProperty("translationManager", &m_TranslationManager);
    context->setContextProperty("uiManager", &m_UIManager);
    context->setContextProperty("quickBuffer", &m_QuickBuffer);
    context->setContextProperty("userDictEditModel", &m_UserDictEditModel);
    context->setContextProperty("switcher", &m_SwitcherModel);
    context->setContextProperty("duplicatesModel", &m_DuplicatesModel);
    context->setContextProperty("csvExportModel", &m_CsvExportModel);
    context->setContextProperty("presetsGroups", m_PresetsModel.getGroupsModel());

    context->setContextProperty("tabsModel", m_UIManager.getTabsModel());
    context->setContextProperty("activeTabs", m_UIManager.getActiveTabs());
    context->setContextProperty("inactiveTabs", m_UIManager.getInactiveTabs());

    context->setContextProperty("xpiksApp", this);

#ifdef QT_DEBUG
    QVariant isDebug(true);
#else
    QVariant isDebug(false);
#endif
    context->setContextProperty("debug", isDebug);
    context->setContextProperty("debugTabs", isDebug);

    m_UIManager.addSystemTab(FILES_FOLDERS_TAB_ID, "qrc:/CollapserTabs/FilesFoldersIcon.qml", "qrc:/CollapserTabs/FilesFoldersTab.qml");
    m_UIManager.addSystemTab(QUICKBUFFER_TAB_ID, "qrc:/CollapserTabs/QuickBufferIcon.qml", "qrc:/CollapserTabs/QuickBufferTab.qml");
    m_UIManager.addSystemTab(TRANSLATOR_TAB_ID, "qrc:/CollapserTabs/TranslatorIcon.qml", "qrc:/CollapserTabs/TranslatorTab.qml");
    m_UIManager.initializeSystemTabs();
    m_UIManager.initialize();
}

void XpiksApp::start() {
    if (m_AfterInitCalled) {
        LOG_WARNING << "Attempt to call afterConstructionCallback() second time";
        return;
    }

    m_RequestsService.startService();

    m_SwitcherModel.initialize();
    m_SwitcherModel.updateConfigs();

    const int waitSeconds = 5;
    Helpers::AsyncCoordinatorStarter deferredStarter(&m_InitCoordinator, waitSeconds);
    Q_UNUSED(deferredStarter);

    m_AfterInitCalled = true;
    std::shared_ptr<Common::ServiceStartParams> emptyParams;
    std::shared_ptr<Common::ServiceStartParams> coordinatorParams(
                new Helpers::AsyncCoordinatorStartParams(&m_InitCoordinator));

    bool dbInitialized = m_DatabaseManager.initialize();
    Q_ASSERT(dbInitialized);
    if (!dbInitialized) {
        LOG_WARNING << "Failed to initialize the DB. Xpiks will crash soon";
    }

    m_MaintenanceService.startService();
    m_ImageCachingService.startService(coordinatorParams);
    m_VideoCachingService.startService();
    m_MetadataIOService.startService();

    m_SpellCheckerService.startService(coordinatorParams);
    m_WarningsService.startService(emptyParams);
    m_AutoCompleteService.startService(coordinatorParams);
    m_TranslationService.startService(coordinatorParams);

    QCoreApplication::processEvents();

    const QString reportingEndpoint =
        QLatin1String(
            "cc39a47f60e1ed812e2403b33678dd1c529f1cc43f66494998ec478a4d13496269a3dfa01f882941766dba246c76b12b2a0308e20afd84371c41cf513260f8eb8b71f8c472cafb1abf712c071938ec0791bbf769ab9625c3b64827f511fa3fbb");
    QString endpoint = Encryption::decodeText(reportingEndpoint, "reporting");
    m_TelemetryService.setEndpoint(endpoint);

    m_TelemetryService.startReporting();
    m_UploadInfoRepository.initializeStocksList(&m_InitCoordinator);
    m_WarningsService.initWarningsSettings();
    m_TranslationManager.initializeDictionaries();
    m_UploadInfoRepository.initializeConfig();
    m_PresetsModel.initializePresets();
    m_CsvExportModel.initializeExportPlans(&m_InitCoordinator);
    m_KeywordsSuggestor.initSuggestionEngines();
    m_UpdateService.initialize();

    m_MainDelegator.readSession();
}

void XpiksApp::stop() {
    LOG_DEBUG << "Shutting down...";
    if (!m_AfterInitCalled) {
        return;
    }

#ifdef WITH_PLUGINS
    m_PluginManager.unloadPlugins();
#endif

    m_MainDelegator.clearCurrentItem();

    m_ArtworksRepository.stopListeningToUnavailableFiles();

    m_ArtItemsModel.disconnect();
    m_ArtItemsModel.deleteAllItems();
    m_FilteredArtItemsModel.disconnect();

    m_ImageCachingService.stopService();
    m_VideoCachingService.stopService();
    m_UpdateService.stopChecking();
    m_MetadataIOService.stopService();

    m_SpellCheckerService.stopService();
    m_WarningsService.stopService();
    m_AutoCompleteService.stopService();
    m_TranslationService.stopService();

    m_SettingsModel.syncronizeSettings();

    m_MaintenanceService.stopService();
    m_DatabaseManager.prepareToFinalize();

    // we have a second for important? stuff
    m_TelemetryService.reportAction(Connectivity::UserAction::Close);
    m_TelemetryService.stopReporting();
    m_RequestsService.stopService();
}

void XpiksApp::shutdown() {
    LOG_DEBUG << "emitting signal";
    emit globalBeforeDestruction();
    QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
    {
        stop();
    }
    QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

    m_UpdateService.tryToUpgradeXpiks();
}

void XpiksApp::upgradeNow() {
    LOG_DEBUG << "#";
    m_UpdateService.setHaveUpgradeConsent();
    emit upgradeInitiated();
}

void XpiksApp::ensureUserIdExists() {
    QString userID = m_SettingsModel.getUserAgentId();
    QUuid latest(userID);
    if (userID.isEmpty()
            || (latest.isNull())
            || (latest.version() == QUuid::VerUnknown)) {
        QUuid uuid = QUuid::createUuid();
        m_SettingsModel.setUserAgentId(uuid.toString());
    }
}

void XpiksApp::injectDependencies() {
    m_MainDelegator.setCommandManager(&m_CommandManager);
    m_CommandManager.InjectDependency(&m_MainDelegator);

    m_CommandManager.InjectDependency(&m_ArtworksRepository);
    m_CommandManager.InjectDependency(&m_ArtItemsModel);
    m_CommandManager.InjectDependency(&m_FilteredArtItemsModel);
    m_CommandManager.InjectDependency(&m_CombinedArtworksModel);
    m_CommandManager.InjectDependency(&m_ArtworkUploader);
    m_CommandManager.InjectDependency(&m_UploadInfoRepository);
    m_CommandManager.InjectDependency(&m_WarningsService);
    m_CommandManager.InjectDependency(&m_SecretsManager);
    m_CommandManager.InjectDependency(&m_UndoRedoManager);
    m_CommandManager.InjectDependency(&m_ZipArchiver);
    m_CommandManager.InjectDependency(&m_KeywordsSuggestor);
    m_CommandManager.InjectDependency(&m_SettingsModel);
    m_CommandManager.InjectDependency(&m_RecentDirectorieModel);
    m_CommandManager.InjectDependency(&m_RecentFileModel);
    m_CommandManager.InjectDependency(&m_SpellCheckerService);
    m_CommandManager.InjectDependency(&m_SpellCheckSuggestionModel);
    m_CommandManager.InjectDependency(&m_MetadataIOService);
    m_CommandManager.InjectDependency(&m_TelemetryService);
    m_CommandManager.InjectDependency(&m_UpdateService);
    m_CommandManager.InjectDependency(&m_LogsModel);
    m_CommandManager.InjectDependency(&m_MetadataIOCoordinator);
    m_CommandManager.InjectDependency(&m_PluginManager);
    m_CommandManager.InjectDependency(&m_LanguagesModel);
    m_CommandManager.InjectDependency(&m_ColorsModel);
    m_CommandManager.InjectDependency(&m_AutoCompleteService);
    m_CommandManager.InjectDependency(&m_AutoCompleteModel);
    m_CommandManager.InjectDependency(&m_ImageCachingService);
    m_CommandManager.InjectDependency(&m_ReplaceModel);
    m_CommandManager.InjectDependency(&m_DeleteKeywordsModel);
    m_CommandManager.InjectDependency(&m_HelpersQmlWrapper);
    m_CommandManager.InjectDependency(&m_PresetsModel);
    m_CommandManager.InjectDependency(&m_TranslationManager);
    m_CommandManager.InjectDependency(&m_TranslationService);
    m_CommandManager.InjectDependency(&m_UIManager);
    m_CommandManager.InjectDependency(&m_ArtworkProxyModel);
    m_CommandManager.InjectDependency(&m_SessionManager);
    m_CommandManager.InjectDependency(&m_WarningsModel);
    m_CommandManager.InjectDependency(&m_QuickBuffer);
    m_CommandManager.InjectDependency(&m_MaintenanceService);
    m_CommandManager.InjectDependency(&m_VideoCachingService);
    m_CommandManager.InjectDependency(&m_ArtworksUpdateHub);
    m_CommandManager.InjectDependency(&m_SwitcherModel);
    m_CommandManager.InjectDependency(&m_RequestsService);
    m_CommandManager.InjectDependency(&m_DatabaseManager);
    m_CommandManager.InjectDependency(&m_DuplicatesModel);
    m_CommandManager.InjectDependency(&m_CsvExportModel);

    m_UserDictEditModel.setCommandManager(&m_CommandManager);

    m_CommandManager.ensureDependenciesInjected();
}

void XpiksApp::afterServicesStarted() {
    LOG_DEBUG << "#";

#ifdef WITH_PLUGINS
    m_PluginManager.loadPlugins();
#endif

    int newFilesAdded = m_MainDelegator.restoreReadSession();
    if (newFilesAdded > 0) {
        // immediately save restored session - to beat race between
        // saving session from Add Command and restoring FULL_DIR flag
        m_MainDelegator.saveSessionInBackground();
    }

#if !defined(INTEGRATION_TESTS)
    m_SwitcherModel.afterInitializedCallback();
#endif

    m_UpdateService.startChecking();

    executeMaintenanceJobs();
}

void XpiksApp::executeMaintenanceJobs() {
    // integration test just don't have old ones
    m_MetadataIOCoordinator.autoDiscoverExiftool();

    m_MaintenanceService.moveSettings(&m_SettingsModel);
#ifdef QT_DEBUG
    m_MaintenanceService.upgradeImagesCache(&m_ImageCachingService);
#endif

    m_MaintenanceService.cleanupLogs();
    m_MaintenanceService.cleanupUpdatesArtifacts();
}

void XpiksApp::connectEntitiesSignalsSlots() {
    QObject::connect(&m_SecretsManager, &Encryption::SecretsManager::beforeMasterPasswordChange,
                     &m_UploadInfoRepository, &Models::UploadInfoRepository::onBeforeMasterPasswordChanged);
    QObject::connect(&m_SecretsManager, &Encryption::SecretsManager::afterMasterPasswordReset,
                     &m_UploadInfoRepository, &Models::UploadInfoRepository::onAfterMasterPasswordReset);

    QObject::connect(&m_ArtItemsModel, &Models::ArtItemsModel::selectedArtworksRemoved,
                     &m_FilteredArtItemsModel, &Models::FilteredArtItemsProxyModel::onSelectedArtworksRemoved);
    QObject::connect(&m_ArtItemsModel, &Models::ArtItemsModel::artworkSelectedChanged,
                     &m_FilteredArtItemsModel, &Models::FilteredArtItemsProxyModel::itemSelectedChanged);

    QObject::connect(&m_SettingsModel, &Models::SettingsModel::settingsUpdated,
                     &m_FilteredArtItemsModel, &Models::FilteredArtItemsProxyModel::onSettingsUpdated);

    QObject::connect(&m_SpellCheckerService, &SpellCheck::SpellCheckerService::serviceAvailable,
                     &m_FilteredArtItemsModel, &Models::FilteredArtItemsProxyModel::onSpellCheckerAvailable);

    QObject::connect(&m_ArtworksRepository, &Models::ArtworksRepository::filesUnavailable,
                     &m_ArtItemsModel, &Models::ArtItemsModel::onFilesUnavailableHandler);

    QObject::connect(&m_UndoRedoManager, &UndoRedo::UndoRedoManager::undoStackEmpty,
                     &m_ArtItemsModel, &Models::ArtItemsModel::onUndoStackEmpty);

    QObject::connect(&m_UndoRedoManager, &UndoRedo::UndoRedoManager::undoStackEmpty,
                     &m_ArtworksRepository, &Models::ArtworksRepository::onUndoStackEmpty);

    QObject::connect(&m_SettingsModel, &Models::SettingsModel::userStatisticsChanged,
                     &m_TelemetryService, &Connectivity::TelemetryService::changeReporting);

    QObject::connect(&m_LanguagesModel, &Models::LanguagesModel::languageChanged,
                     &m_KeywordsSuggestor, &Suggestion::KeywordsSuggestor::onLanguageChanged);

    QObject::connect(&m_UpdateService, &Connectivity::UpdateService::updateAvailable,
                     &m_HelpersQmlWrapper, &Helpers::HelpersQmlWrapper::updateAvailable);
    QObject::connect(&m_UpdateService, &Connectivity::UpdateService::updateDownloaded,
                     this, &XpiksApp::isUpdateDownloadedChanged);
    QObject::connect(&m_UpdateService, &Connectivity::UpdateService::updateDownloaded,
                     &m_HelpersQmlWrapper, &Helpers::HelpersQmlWrapper::updateDownloaded);

    QObject::connect(&m_WarningsService, &Warnings::WarningsService::queueIsEmpty,
                     &m_WarningsModel, &Warnings::WarningsModel::onWarningsUpdateRequired);

    QObject::connect(&m_MetadataIOCoordinator, &MetadataIO::MetadataIOCoordinator::recommendedExiftoolFound,
                     &m_SettingsModel, &Models::SettingsModel::onRecommendedExiftoolFound);

    QObject::connect(&m_MetadataIOCoordinator, &MetadataIO::MetadataIOCoordinator::metadataReadingFinished,
                     &m_ArtItemsModel, &Models::ArtItemsModel::modifiedArtworksCountChanged);

    QObject::connect(&m_MetadataIOCoordinator, &MetadataIO::MetadataIOCoordinator::metadataWritingFinished,
                     &m_ArtItemsModel, &Models::ArtItemsModel::modifiedArtworksCountChanged);

    QObject::connect(&m_SpellCheckerService, &SpellCheck::SpellCheckerService::userDictUpdate,
                     &m_ArtItemsModel, &Models::ArtItemsModel::userDictUpdateHandler);
    QObject::connect(&m_SpellCheckerService, &SpellCheck::SpellCheckerService::userDictCleared,
                     &m_ArtItemsModel, &Models::ArtItemsModel::userDictClearedHandler);

    QObject::connect(&m_SpellCheckerService, &SpellCheck::SpellCheckerService::userDictUpdate,
                     &m_CombinedArtworksModel, &Models::CombinedArtworksModel::userDictUpdateHandler);
    QObject::connect(&m_SpellCheckerService, &SpellCheck::SpellCheckerService::userDictCleared,
                     &m_CombinedArtworksModel, &Models::CombinedArtworksModel::userDictClearedHandler);

    QObject::connect(&m_ArtItemsModel, &Models::ArtItemsModel::fileWithIndexUnavailable,
                     &m_ArtworkProxyModel, &Models::ArtworkProxyModel::itemUnavailableHandler);

    QObject::connect(&m_SpellCheckerService, &SpellCheck::SpellCheckerService::userDictUpdate,
                     &m_ArtworkProxyModel, &Models::ArtworkProxyModel::userDictUpdateHandler);
    QObject::connect(&m_SpellCheckerService, &SpellCheck::SpellCheckerService::userDictCleared,
                     &m_ArtworkProxyModel, &Models::ArtworkProxyModel::userDictClearedHandler);

    QObject::connect(&m_ArtworkProxyModel, &Models::ArtworkProxyModel::warningsCouldHaveChanged,
                     &m_WarningsModel, &Warnings::WarningsModel::onWarningsCouldHaveChanged);

    QObject::connect(&m_ArtworkProxyModel, &Models::ArtworkProxyModel::duplicatesCouldHaveChanged,
                     &m_DuplicatesModel, &SpellCheck::DuplicatesReviewModel::onDuplicatesCouldHaveChanged);

    QObject::connect(&m_SpellCheckerService, &SpellCheck::SpellCheckerService::userDictUpdate,
                     &m_QuickBuffer, &QuickBuffer::QuickBuffer::userDictUpdateHandler);
    QObject::connect(&m_SpellCheckerService, &SpellCheck::SpellCheckerService::userDictCleared,
                     &m_QuickBuffer, &QuickBuffer::QuickBuffer::userDictClearedHandler);

#ifdef WITH_PLUGINS
    QObject::connect(&m_UIManager, &Models::UIManager::currentEditableChanged,
                     &m_PluginManager, &Plugins::PluginManager::onCurrentEditableChanged);

    QObject::connect(&m_UndoRedoManager, &UndoRedo::UndoRedoManager::actionUndone,
                     &m_PluginManager, &Plugins::PluginManager::onLastActionUndone);

    QObject::connect(&m_PresetsModel, &KeywordsPresets::PresetKeywordsModel::presetsUpdated,
                     &m_PluginManager, &Plugins::PluginManager::onPresetsUpdated);
#endif
}

void XpiksApp::servicesInitialized(int status) {
    LOG_DEBUG << "#";
    Q_ASSERT(m_ServicesInitialized == false);

    Helpers::AsyncCoordinator::CoordinationStatus coordStatus = (Helpers::AsyncCoordinator::CoordinationStatus)status;

    if (m_ServicesInitialized == false) {
        m_ServicesInitialized = true;

        if (coordStatus == Helpers::AsyncCoordinator::AllDone ||
                coordStatus == Helpers::AsyncCoordinator::Timeout) {
            afterServicesStarted();
        }
    }
}

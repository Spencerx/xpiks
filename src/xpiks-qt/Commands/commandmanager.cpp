/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "commandmanager.h"
#include "../Common/defines.h"
#include "../Models/artworksrepository.h"
#include "../Models/artitemsmodel.h"
#include "../Models/imageartwork.h"
#include "../Models/combinedartworksmodel.h"
#include "../Models/artworkuploader.h"
#include "../Models/uploadinforepository.h"
#include "../Models/uploadinfo.h"
#include "../Models/artworkmetadata.h"
#include "../Encryption/secretsmanager.h"
#include "../UndoRedo/undoredomanager.h"
#include "../Models/ziparchiver.h"
#include "../Suggestion/keywordssuggestor.h"
#include "../Commands/addartworkscommand.h"
#include "../Models/filteredartitemsproxymodel.h"
#include "../Models/recentdirectoriesmodel.h"
#include "../Models/recentfilesmodel.h"
#include "../Models/artworkelement.h"
#include "../SpellCheck/duplicatesreviewmodel.h"
#include "../SpellCheck/spellcheckerservice.h"
#include "../Models/settingsmodel.h"
#include "../SpellCheck/spellchecksuggestionmodel.h"
#include "../MetadataIO/metadataioservice.h"
#include "../Connectivity/telemetryservice.h"
#include "../Connectivity/updateservice.h"
#include "../Models/logsmodel.h"
#include "../Encryption/aes-qt.h"
#include "../MetadataIO/metadataiocoordinator.h"
#include "../Plugins/pluginmanager.h"
#include "../Warnings/warningsservice.h"
#include "../Models/languagesmodel.h"
#include "../AutoComplete/autocompleteservice.h"
#include "../QMLExtensions/imagecachingservice.h"
#include "../Models/findandreplacemodel.h"
#include "../Models/deletekeywordsviewmodel.h"
#include "../Helpers/helpersqmlwrapper.h"
#include "../Helpers/updatehelpers.h"
#include "../Common/imetadataoperator.h"
#include "../Translation/translationmanager.h"
#include "../Translation/translationservice.h"
#include "../Models/uimanager.h"
#include "../Models/artworkproxymodel.h"
#include "../Models/sessionmanager.h"
#include "../Warnings/warningsmodel.h"
#include "../QuickBuffer/quickbuffer.h"
#include "../QuickBuffer/currenteditableartwork.h"
#include "../QuickBuffer/currenteditableproxyartwork.h"
#include "../Maintenance/maintenanceservice.h"
#include "../QMLExtensions/videocachingservice.h"
#include "../QMLExtensions/artworksupdatehub.h"
#include "../Helpers/asynccoordinator.h"
#include "../Storage/database.h"
#include "../Storage/databasemanager.h"
#include "../Models/switchermodel.h"
#include "../Connectivity/requestsservice.h"
#include "../AutoComplete/keywordsautocompletemodel.h"
#include "../MetadataIO/csvexportmodel.h"

Commands::CommandManager::CommandManager():
    m_ArtworksRepository(NULL),
    m_ArtItemsModel(NULL),
    m_FilteredItemsModel(NULL),
    m_CombinedArtworksModel(NULL),
    m_ArtworkUploader(NULL),
    m_UploadInfoRepository(NULL),
    m_WarningsService(NULL),
    m_SecretsManager(NULL),
    m_UndoRedoManager(NULL),
    m_ZipArchiver(NULL),
    m_KeywordsSuggestor(NULL),
    m_SettingsModel(NULL),
    m_RecentDirectories(NULL),
    m_RecentFiles(NULL),
    m_SpellCheckerService(NULL),
    m_SpellCheckSuggestionModel(NULL),
    m_MetadataIOService(NULL),
    m_TelemetryService(NULL),
    m_UpdateService(NULL),
    m_LogsModel(NULL),
    m_MetadataIOCoordinator(NULL),
    m_PluginManager(NULL),
    m_LanguagesModel(NULL),
    m_ColorsModel(NULL),
    m_AutoCompleteService(NULL),
    m_AutoCompleteModel(NULL),
    m_ImageCachingService(NULL),
    m_DeleteKeywordsViewModel(NULL),
    m_FindAndReplaceModel(NULL),
    m_HelpersQmlWrapper(NULL),
    m_PresetsModel(NULL),
    m_TranslationService(NULL),
    m_TranslationManager(NULL),
    m_UIManager(NULL),
    m_ArtworkProxyModel(NULL),
    m_SessionManager(NULL),
    m_WarningsModel(NULL),
    m_QuickBuffer(NULL),
    m_MaintenanceService(NULL),
    m_VideoCachingService(NULL),
    m_ArtworksUpdateHub(NULL),
    m_SwitcherModel(NULL),
    m_RequestsService(NULL),
    m_DuplicatesModel(NULL),
    m_CsvExportModel(NULL),
    m_DatabaseManager(NULL),
    m_MainDelegator(NULL),
    m_LastCommandID(0)
{

}

void Commands::CommandManager::InjectDependency(Models::ArtworksRepository *artworkRepository) {
    Q_ASSERT(artworkRepository != NULL); m_ArtworksRepository = artworkRepository;
}

void Commands::CommandManager::InjectDependency(Models::ArtItemsModel *artItemsModel) {
    Q_ASSERT(artItemsModel != NULL); m_ArtItemsModel = artItemsModel;
    m_ArtItemsModel->setCommandManager(this);
    // do not add to availabilityListeners
}

void Commands::CommandManager::InjectDependency(Models::FilteredArtItemsProxyModel *filteredItemsModel) {
    Q_ASSERT(filteredItemsModel != NULL); m_FilteredItemsModel = filteredItemsModel;
    m_FilteredItemsModel->setCommandManager(this);
}

void Commands::CommandManager::InjectDependency(Models::CombinedArtworksModel *combinedArtworksModel) {
    Q_ASSERT(combinedArtworksModel != NULL); m_CombinedArtworksModel = combinedArtworksModel;
    m_CombinedArtworksModel->setCommandManager(this);
    m_MainDelegator->addAvailabilityListener(combinedArtworksModel);
}

void Commands::CommandManager::InjectDependency(Models::ArtworkUploader *artworkUploader) {
    Q_ASSERT(artworkUploader != NULL); m_ArtworkUploader = artworkUploader;
    m_ArtworkUploader->setCommandManager(this);
    m_MainDelegator->addAvailabilityListener(m_ArtworkUploader);
}

void Commands::CommandManager::InjectDependency(Models::UploadInfoRepository *uploadInfoRepository) {
    Q_ASSERT(uploadInfoRepository != NULL); m_UploadInfoRepository = uploadInfoRepository;
    m_UploadInfoRepository->setCommandManager(this);
}

void Commands::CommandManager::InjectDependency(Warnings::WarningsService *warningsService) {
    Q_ASSERT(warningsService != NULL); m_WarningsService = warningsService;
    m_WarningsService->setCommandManager(this);
}

void Commands::CommandManager::InjectDependency(Encryption::SecretsManager *secretsManager) {
    Q_ASSERT(secretsManager != NULL); m_SecretsManager = secretsManager;
    m_SecretsManager->setCommandManager(this);
}

void Commands::CommandManager::InjectDependency(UndoRedo::UndoRedoManager *undoRedoManager) {
    Q_ASSERT(undoRedoManager != NULL); m_UndoRedoManager = undoRedoManager;
    m_UndoRedoManager->setCommandManager(this);
}

void Commands::CommandManager::InjectDependency(Models::ZipArchiver *zipArchiver) {
    Q_ASSERT(zipArchiver != NULL); m_ZipArchiver = zipArchiver;
    m_ZipArchiver->setCommandManager(this);
    m_MainDelegator->addAvailabilityListener(zipArchiver);
}

void Commands::CommandManager::InjectDependency(Suggestion::KeywordsSuggestor *keywordsSuggestor) {
    Q_ASSERT(keywordsSuggestor != NULL); m_KeywordsSuggestor = keywordsSuggestor;
    m_KeywordsSuggestor->setCommandManager(this);
}

void Commands::CommandManager::InjectDependency(Models::SettingsModel *settingsModel) {
    Q_ASSERT(settingsModel != NULL); m_SettingsModel = settingsModel;
    m_SettingsModel->setCommandManager(this);
}

void Commands::CommandManager::InjectDependency(Models::RecentDirectoriesModel *recentDirectories) {
    Q_ASSERT(recentDirectories != NULL); m_RecentDirectories = recentDirectories;
}

void Commands::CommandManager::InjectDependency(Models::RecentFilesModel *recentFiles) {
    Q_ASSERT(recentFiles != NULL); m_RecentFiles = recentFiles;
}

void Commands::CommandManager::InjectDependency(SpellCheck::SpellCheckerService *spellCheckerService) {
    Q_ASSERT(spellCheckerService != NULL); m_SpellCheckerService = spellCheckerService;
}

void Commands::CommandManager::InjectDependency(SpellCheck::SpellCheckSuggestionModel *spellCheckSuggestionModel) {
    Q_ASSERT(spellCheckSuggestionModel != NULL); m_SpellCheckSuggestionModel = spellCheckSuggestionModel;
    m_SpellCheckSuggestionModel->setCommandManager(this);
}

void Commands::CommandManager::InjectDependency(MetadataIO::MetadataIOService *metadataIOService) {
    Q_ASSERT(metadataIOService != NULL); m_MetadataIOService = metadataIOService;
    m_MetadataIOService->setCommandManager(this);
}

void Commands::CommandManager::InjectDependency(Connectivity::TelemetryService *telemetryService) {
    Q_ASSERT(telemetryService != NULL); m_TelemetryService = telemetryService;
}

void Commands::CommandManager::InjectDependency(Connectivity::UpdateService *updateService) {
    Q_ASSERT(updateService != NULL); m_UpdateService = updateService;
}

void Commands::CommandManager::InjectDependency(Models::LogsModel *logsModel) {
    Q_ASSERT(logsModel != NULL); m_LogsModel = logsModel;
}

void Commands::CommandManager::InjectDependency(MetadataIO::MetadataIOCoordinator *metadataIOCoordinator) {
    Q_ASSERT(metadataIOCoordinator != NULL); m_MetadataIOCoordinator = metadataIOCoordinator;
    m_MetadataIOCoordinator->setCommandManager(this);
}

void Commands::CommandManager::InjectDependency(Plugins::PluginManager *pluginManager) {
    Q_ASSERT(pluginManager != NULL); m_PluginManager = pluginManager;
    m_PluginManager->setCommandManager(this);
}

void Commands::CommandManager::InjectDependency(Models::LanguagesModel *languagesModel) {
    Q_ASSERT(languagesModel != NULL); m_LanguagesModel = languagesModel;
    m_LanguagesModel->setCommandManager(this);
}

void Commands::CommandManager::InjectDependency(QMLExtensions::ColorsModel *colorsModel) {
    Q_ASSERT(colorsModel != NULL); m_ColorsModel = colorsModel;
}

void Commands::CommandManager::InjectDependency(AutoComplete::AutoCompleteService *autoCompleteService) {
    Q_ASSERT(autoCompleteService != NULL); m_AutoCompleteService = autoCompleteService;
}

void Commands::CommandManager::InjectDependency(AutoComplete::KeywordsAutoCompleteModel *autoCompleteModel) {
    Q_ASSERT(autoCompleteModel != NULL); m_AutoCompleteModel = autoCompleteModel;
    m_AutoCompleteModel->setCommandManager(this);
}

void Commands::CommandManager::InjectDependency(QMLExtensions::ImageCachingService *imageCachingService) {
    Q_ASSERT(imageCachingService != NULL); m_ImageCachingService = imageCachingService;
    m_ImageCachingService->setCommandManager(this);
}

void Commands::CommandManager::InjectDependency(Models::FindAndReplaceModel *findAndReplaceModel) {
    Q_ASSERT(findAndReplaceModel != NULL); m_FindAndReplaceModel = findAndReplaceModel;
    m_FindAndReplaceModel->setCommandManager(this);
}

void Commands::CommandManager::InjectDependency(Models::DeleteKeywordsViewModel *deleteKeywordsViewModel) {
    Q_ASSERT(deleteKeywordsViewModel != NULL); m_DeleteKeywordsViewModel = deleteKeywordsViewModel;
    m_DeleteKeywordsViewModel->setCommandManager(this);
}

void Commands::CommandManager::InjectDependency(Helpers::HelpersQmlWrapper *helpersQmlWrapper) {
    Q_ASSERT(helpersQmlWrapper != NULL); m_HelpersQmlWrapper = helpersQmlWrapper;
    m_HelpersQmlWrapper->setCommandManager(this);
}

void Commands::CommandManager::InjectDependency(KeywordsPresets::PresetKeywordsModel *presetsModel) {
    Q_ASSERT(presetsModel != NULL); m_PresetsModel = presetsModel;
    m_PresetsModel->setCommandManager(this);
}

void Commands::CommandManager::InjectDependency(Translation::TranslationService *translationService) {
    Q_ASSERT(translationService != NULL); m_TranslationService = translationService;
}

void Commands::CommandManager::InjectDependency(Translation::TranslationManager *translationManager) {
    Q_ASSERT(translationManager != NULL); m_TranslationManager = translationManager;
    m_TranslationManager->setCommandManager(this);
}

void Commands::CommandManager::InjectDependency(Models::UIManager *uiManager) {
    Q_ASSERT(uiManager != NULL); m_UIManager = uiManager;
}

void Commands::CommandManager::InjectDependency(Models::ArtworkProxyModel *artworkProxy) {
    Q_ASSERT(artworkProxy != NULL); m_ArtworkProxyModel = artworkProxy;
    m_ArtworkProxyModel->setCommandManager(this);
}

void Commands::CommandManager::InjectDependency(Models::SessionManager *sessionManager) {
    Q_ASSERT(sessionManager != NULL); m_SessionManager = sessionManager;
    m_SessionManager->setCommandManager(this);
}

void Commands::CommandManager::InjectDependency(Warnings::WarningsModel *warningsModel) {
    Q_ASSERT(warningsModel != NULL); m_WarningsModel = warningsModel;
}

void Commands::CommandManager::InjectDependency(QuickBuffer::QuickBuffer *quickBuffer) {
    Q_ASSERT(quickBuffer != NULL); m_QuickBuffer = quickBuffer;
    m_QuickBuffer->setCommandManager(this);
}

void Commands::CommandManager::InjectDependency(Maintenance::MaintenanceService *maintenanceService) {
    Q_ASSERT(maintenanceService != NULL); m_MaintenanceService = maintenanceService;
}

void Commands::CommandManager::InjectDependency(QMLExtensions::VideoCachingService *videoCachingService) {
    Q_ASSERT(videoCachingService != NULL); m_VideoCachingService = videoCachingService;
    m_VideoCachingService->setCommandManager(this);
}

void Commands::CommandManager::InjectDependency(Models::SwitcherModel *switcherModel) {
    Q_ASSERT(switcherModel != NULL); m_SwitcherModel = switcherModel;
    m_SwitcherModel->setCommandManager(this);
}

void Commands::CommandManager::InjectDependency(QMLExtensions::ArtworksUpdateHub *artworksUpdateHub) {
    Q_ASSERT(artworksUpdateHub != NULL); m_ArtworksUpdateHub = artworksUpdateHub;
    m_ArtworksUpdateHub->setCommandManager(this);
}

void Commands::CommandManager::InjectDependency(Connectivity::RequestsService *requestsService) {
    Q_ASSERT(requestsService != NULL); m_RequestsService = requestsService;
}

void Commands::CommandManager::InjectDependency(Storage::DatabaseManager *databaseManager) {
    Q_ASSERT(databaseManager != NULL); m_DatabaseManager = databaseManager;
}

void Commands::CommandManager::InjectDependency(SpellCheck::DuplicatesReviewModel *duplicatesModel) {
    Q_ASSERT(duplicatesModel != NULL); m_DuplicatesModel = duplicatesModel;
}

void Commands::CommandManager::InjectDependency(MetadataIO::CsvExportModel *csvExportModel) {
    Q_ASSERT(csvExportModel != NULL); m_CsvExportModel = csvExportModel;
    m_CsvExportModel->setCommandManager(this);
}

void Commands::CommandManager::InjectDependency(MainDelegator *delegator) {
    m_MainDelegator = delegator;
}

std::shared_ptr<Commands::ICommandResult> Commands::CommandManager::processCommand(const std::shared_ptr<ICommandBase> &command) {
    Q_ASSERT(command);
    const int id = generateNextCommandID();
    std::shared_ptr<Commands::ICommandResult> result = command->execute(id);
    return result;
}

void Commands::CommandManager::ensureDependenciesInjected() {
    Q_ASSERT(m_ArtworksRepository != NULL);
    Q_ASSERT(m_ArtItemsModel != NULL);
    Q_ASSERT(m_FilteredItemsModel != NULL);
    Q_ASSERT(m_CombinedArtworksModel != NULL);
    Q_ASSERT(m_ArtworkUploader != NULL);
    Q_ASSERT(m_UploadInfoRepository != NULL);
    Q_ASSERT(m_WarningsService != NULL);
    Q_ASSERT(m_SecretsManager != NULL);
    Q_ASSERT(m_UndoRedoManager != NULL);
    Q_ASSERT(m_ZipArchiver != NULL);
    Q_ASSERT(m_KeywordsSuggestor != NULL);
    Q_ASSERT(m_SettingsModel != NULL);
    Q_ASSERT(m_RecentDirectories != NULL);
    Q_ASSERT(m_RecentFiles != NULL);
    Q_ASSERT(m_SpellCheckerService != NULL);
    Q_ASSERT(m_SpellCheckSuggestionModel != NULL);
    Q_ASSERT(m_MetadataIOService != NULL);
    Q_ASSERT(m_TelemetryService != NULL);
    Q_ASSERT(m_UpdateService != NULL);
    Q_ASSERT(m_LogsModel != NULL);
    Q_ASSERT(m_MetadataIOCoordinator != NULL);
    Q_ASSERT(m_PluginManager != NULL);
    Q_ASSERT(m_LanguagesModel != NULL);
    Q_ASSERT(m_ColorsModel != NULL);
    Q_ASSERT(m_AutoCompleteService != NULL);
    Q_ASSERT(m_ImageCachingService != NULL);
    Q_ASSERT(m_FindAndReplaceModel != NULL);
    Q_ASSERT(m_DeleteKeywordsViewModel != NULL);
    Q_ASSERT(m_PresetsModel != NULL);
    Q_ASSERT(m_TranslationService != NULL);
    Q_ASSERT(m_TranslationManager != NULL);
    Q_ASSERT(m_ArtworkProxyModel != NULL);
    Q_ASSERT(m_SessionManager != NULL);
    Q_ASSERT(m_WarningsModel != NULL);
    Q_ASSERT(m_QuickBuffer != NULL);
    Q_ASSERT(m_MaintenanceService != NULL);
    Q_ASSERT(m_VideoCachingService != NULL);
    Q_ASSERT(m_RequestsService != NULL);
    Q_ASSERT(m_ArtworksUpdateHub != NULL);
    Q_ASSERT(m_DuplicatesModel != NULL);

#if !defined(CORE_TESTS)
    Q_ASSERT(m_DatabaseManager != NULL);
    Q_ASSERT(m_AutoCompleteModel != NULL);
    Q_ASSERT(m_CsvExportModel != NULL);
    Q_ASSERT(m_SwitcherModel != NULL);
#endif

#if !defined(INTEGRATION_TESTS)
    Q_ASSERT(m_HelpersQmlWrapper != NULL);
#endif

#if !defined(INTEGRATION_TESTS) && !defined(CORE_TESTS)
    Q_ASSERT(m_UIManager != NULL);
#endif
}


#ifdef INTEGRATION_TESTS
void Commands::CommandManager::cleanup() {
    LOG_INTEGRATION_TESTS << "#";
    m_SpellCheckerService->cancelCurrentBatch();
    m_WarningsService->cancelCurrentBatch();
    m_MaintenanceService->cleanup();
    m_ArtworksUpdateHub->clear();
    m_AutoCompleteModel->clear();

    m_CsvExportModel->clearModel();
    m_CsvExportModel->resetModel();
    m_CombinedArtworksModel->resetModel();
    m_ZipArchiver->resetModel();
    m_ArtworkUploader->resetModel();
    m_ArtworksRepository->resetEverything();
    m_ArtItemsModel->deleteAllItems();
    m_SettingsModel->resetToDefault();
    m_SpellCheckerService->clearUserDictionary();
    m_SessionManager->clearSession();
    m_MetadataIOCoordinator->clear();
}
#endif

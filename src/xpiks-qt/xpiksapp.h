/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir "kushnirTV@gmail.com"
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef XPIKSAPP_H
#define XPIKSAPP_H

#include <Commands/commandmanager.h>

#include <Common/isystemenvironment.h>

#include <Models/Artworks/artworkslistmodel.h>
#include <Models/Editing/artworkproxymodel.h>
#include <Models/Artworks/artworksrepository.h>
#include <Models/Connectivity/artworkuploader.h>
#include <Models/Editing/combinedartworksmodel.h>
#include <Models/Editing/deletekeywordsviewmodel.h>
#include <Models/Artworks/filteredartworkslistmodel.h>
#include <Models/Editing/findandreplacemodel.h>
#include <Models/languagesmodel.h>
#include <Models/logsmodel.h>
#include <Models/Session/recentdirectoriesmodel.h>
#include <Models/Session/recentfilesmodel.h>
#include <Models/Session/sessionmanager.h>
#include <Models/settingsmodel.h>
#include <Models/switchermodel.h>
#include <Models/uimanager.h>
#include <Models/Connectivity/ziparchiver.h>
#include <Models/Editing/currenteditablemodel.h>
#include <Models/Editing/quickbuffer.h>

#include <Connectivity/requestsservice.h>
#include <Connectivity/telemetryservice.h>
#include <Connectivity/updateservice.h>

#include <Services/Warnings/warningsmodel.h>
#include <Services/Warnings/warningsservice.h>
#include <Services/Warnings/warningssettingsmodel.h>
#include <Services/artworksupdatehub.h>
#include <Services/AutoComplete/autocompleteservice.h>
#include <Services/AutoComplete/keywordsautocompletemodel.h>
#include <Services/Translation/translationmanager.h>
#include <Services/Translation/translationservice.h>
#include <Services/SpellCheck/duplicatesreviewmodel.h>
#include <Services/SpellCheck/spellcheckservice.h>
#include <Services/SpellCheck/spellchecksuggestionmodel.h>
#include <Services/SpellCheck/userdicteditmodel.h>
#include <Services/Maintenance/maintenanceservice.h>
#include <Services/artworksinspectionhub.h>
#include <Services/artworksupdatehub.h>

#include <QMLExtensions/colorsmodel.h>
#include <QMLExtensions/imagecachingservice.h>
#include <QMLExtensions/videocachingservice.h>
#include <QMLExtensions/uicommanddispatcher.h>

#include <UndoRedo/undoredomanager.h>

#include <Storage/databasemanager.h>

#include <Encryption/isecretsstorage.h>

#include <Microstocks/microstockapiclients.h>
#include <Microstocks/stocksftplistmodel.h>

#include <Suggestion/keywordssuggestor.h>

#include <KeywordsPresets/presetkeywordsmodel.h>

#include <ftpcoordinator.h>

#include <MetadataIO/csvexportmodel.h>
#include <MetadataIO/metadataiocoordinator.h>
#include <MetadataIO/metadataioservice.h>
#include <MetadataIO/csvexportplansmodel.h>
#include <MetadataIO/metadatareadinghub.h>

#include <Plugins/pluginmanager.h>
#include <Plugins/uiprovider.h>

#include <Helpers/helpersqmlwrapper.h>
#include <Helpers/ifilenotavailablemodel.h>

class QQmlContext;
class QQuickWindow;

class XpiksApp: public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool pluginsAvailable READ getPluginsAvailable CONSTANT)
    Q_PROPERTY(bool isUpdateDownloaded READ getIsUpdateDownloaded NOTIFY isUpdateDownloadedChanged)
public:
    XpiksApp(Common::ISystemEnvironment &environment);
    virtual ~XpiksApp();

public:
    bool getIsUpdateDownloaded();
    bool getPluginsAvailable() const;

public:
    Plugins::UIProvider &getUIProvider();
    QMLExtensions::ImageCachingService &getImageCachingService() { return m_ImageCachingService; }

public:
    void startLogging();
    virtual void initialize();
    void setupUI(QQmlContext *context);
    void start();
    void stop();
    void setupWindow(QQuickWindow *window);

public:
    Q_INVOKABLE void shutdown();
    Q_INVOKABLE void upgradeNow();
    Q_INVOKABLE void debugCrash();

public:
    Q_INVOKABLE void addFiles(const QList<QUrl> &urls);
    Q_INVOKABLE void addDirectories(const QList<QUrl> &urls);
    Q_INVOKABLE void dropItems(const QList<QUrl> &urls);

public:
    Q_INVOKABLE void removeDirectory(int index);
    Q_INVOKABLE void removeUnavailableFiles();

private:
    void doAddFiles(const std::shared_ptr<Filesystem::IFilesCollection> &files, Common::AddFilesFlags flags);
    void afterServicesStarted();
    void executeMaintenanceJobs();
    void connectEntitiesSignalsSlots();
    void registerUICommands();

signals:
    void globalCloseRequested();
    void globalBeforeDestruction();
    void upgradeInitiated();
    void isUpdateDownloadedChanged();
    void artworksAdded(int importID, int imagesCount, int vectorsCount);

private slots:
    void servicesInitialized(int status);

protected:
    // general
    QMLExtensions::ColorsModel m_ColorsModel;
    Models::LogsModel m_LogsModel;
    Models::SettingsModel m_SettingsModel;
    Storage::DatabaseManager m_DatabaseManager;
    Models::SessionManager m_SessionManager;
    UndoRedo::UndoRedoManager m_UndoRedoManager;
    Commands::CommandManager m_CommandManager;
    Models::SwitcherModel m_SwitcherModel;
    QMLExtensions::UICommandDispatcher m_UICommandDispatcher;

    // models
    KeywordsPresets::PresetKeywordsModel m_PresetsModel;
    KeywordsPresets::FilteredPresetKeywordsModel m_FilteredPresetsModel;
    AutoComplete::KeywordsCompletionsModel m_KeywordsCompletions;
    AutoComplete::KeywordsAutoCompleteModel m_KeywordsAutoCompleteModel;
    Models::CurrentEditableModel m_CurrentEditableModel;

    // artworks
    Models::RecentDirectoriesModel m_RecentDirectorieModel;
    Models::RecentFilesModel m_RecentFileModel;
    Models::ArtworksRepository m_ArtworksRepository;
    Models::FilteredArtworksRepository m_FilteredArtworksRepository;
    Models::ArtworksListModel m_ArtworksListModel;
    Services::ArtworksUpdateHub m_ArtworksUpdateHub;

    // editing
    Models::CombinedArtworksModel m_CombinedArtworksModel;
    Models::QuickBuffer m_QuickBuffer;
    Models::FindAndReplaceModel m_ReplaceModel;
    Models::DeleteKeywordsViewModel m_DeleteKeywordsModel;
    Models::ArtworkProxyModel m_ArtworkProxyModel;
    SpellCheck::DuplicatesReviewModel m_DuplicatesModel;

    // services
    Maintenance::MaintenanceService m_MaintenanceService;
    AutoComplete::AutoCompleteService m_AutoCompleteService;
    Connectivity::RequestsService m_RequestsService;
    Warnings::WarningsSettingsModel m_WarningsSettingsModel;
    Warnings::WarningsService m_WarningsService;
    SpellCheck::SpellCheckService m_SpellCheckerService;
    MetadataIO::MetadataIOService m_MetadataIOService;
    QMLExtensions::ImageCachingService m_ImageCachingService;
    Translation::TranslationService m_TranslationService;
    QMLExtensions::VideoCachingService m_VideoCachingService;
    Connectivity::UpdateService m_UpdateService;
    Connectivity::TelemetryService m_TelemetryService;
    Services::ArtworksInspectionHub m_InspectionHub;

    // connectivity
    Encryption::SecretsManager m_SecretsManager;
    Microstocks::StocksFtpListModel m_StocksFtpList;
    Models::UploadInfoRepository m_UploadInfoRepository;
    Models::ZipArchiver m_ZipArchiver;
    std::shared_ptr<Encryption::ISecretsStorage> m_SecretsStorage;
    Microstocks::MicrostockAPIClients m_ApiClients;
    std::shared_ptr<libxpks::net::FtpCoordinator> m_FtpCoordinator;
    Models::ArtworkUploader m_ArtworkUploader;

    // other
    Models::LanguagesModel m_LanguagesModel;
    Models::UIManager m_UIManager;
    SpellCheck::UserDictionary m_UserDictionary;
    SpellCheck::UserDictEditModel m_UserDictEditModel;
    Warnings::WarningsModel m_WarningsModel;
    Translation::TranslationManager m_TranslationManager;
    MetadataIO::CsvExportPlansModel m_CsvExportPlans;
    MetadataIO::CsvExportModel m_CsvExportModel;
    MetadataIO::MetadataReadingHub m_MetadataReadingHub;
    MetadataIO::MetadataIOCoordinator m_MetadataIOCoordinator;
    SpellCheck::SpellCheckSuggestionModel m_SpellSuggestionModel;

    // dependent
    Models::FilteredArtworksListModel m_FilteredArtworksListModel;
    Suggestion::KeywordsSuggestor m_KeywordsSuggestor;

    // plugins
    Plugins::PluginManager m_PluginManager;
    Plugins::PluginsWithActionsModel m_PluginsWithActions;

    // ui
    Helpers::HelpersQmlWrapper m_HelpersQmlWrapper;

    Helpers::AsyncCoordinator m_InitCoordinator;
    QVector<Helpers::IFileNotAvailableModel*> m_AvailabilityListeners;
    volatile bool m_ServicesInitialized = false;
    volatile bool m_AfterInitCalled = false;
};

#endif // XPIKSAPP_H

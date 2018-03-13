/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SETTINGSMODEL_H
#define SETTINGSMODEL_H

#include <QAbstractListModel>
#include <QObject>
#include <QString>
#include <QCoreApplication>
#include <QDir>
#include <QJsonObject>
#include <QSettings>
#include <QMutex>
#include "../Common/baseentity.h"
#include "../Common/version.h"
#include "../Models/proxysettings.h"
#include "../Helpers/localconfig.h"
#include "../Helpers/constants.h"
#include "../Commands/commandmanager.h"
#include "../Encryption/secretsmanager.h"
#include "../Models/uploadinforepository.h"
#include "../Common/delayedactionentity.h"
#include "../Common/statefulentity.h"
#include "../Common/isystemenvironment.h"
#include "../Helpers/jsonobjectmap.h"

namespace Models {
    class SettingsModel:
            public QObject,
            public Common::BaseEntity,
            public Common::DelayedActionEntity
    {
        Q_OBJECT
        Q_PROPERTY(QString exifToolPath READ getExifToolPath WRITE setExifToolPath NOTIFY exifToolPathChanged)
        Q_PROPERTY(int uploadTimeout READ getUploadTimeout WRITE setUploadTimeout NOTIFY uploadTimeoutChanged)
        Q_PROPERTY(bool mustUseMasterPassword READ getMustUseMasterPassword WRITE setMustUseMasterPassword NOTIFY mustUseMasterPasswordChanged)
        Q_PROPERTY(bool mustUseConfirmations READ getMustUseConfirmations WRITE setMustUseConfirmations NOTIFY mustUseConfirmationsChanged)
        Q_PROPERTY(bool saveSession READ getSaveSession WRITE setSaveSession NOTIFY saveSessionChanged)
        Q_PROPERTY(bool saveBackups READ getSaveBackups WRITE setSaveBackups NOTIFY saveBackupsChanged)
        Q_PROPERTY(double keywordSizeScale READ getKeywordSizeScale WRITE setKeywordSizeScale NOTIFY keywordSizeScaleChanged)
        Q_PROPERTY(int dismissDuration READ getDismissDuration WRITE setDismissDuration NOTIFY dismissDurationChanged)
        Q_PROPERTY(int maxParallelUploads READ getMaxParallelUploads WRITE setMaxParallelUploads NOTIFY maxParallelUploadsChanged)
        Q_PROPERTY(bool fitSmallPreview READ getFitSmallPreview WRITE setFitSmallPreview NOTIFY fitSmallPreviewChanged)
        Q_PROPERTY(bool searchUsingAnd READ getSearchUsingAnd WRITE setSearchUsingAnd NOTIFY searchUsingAndChanged)
        Q_PROPERTY(bool searchByFilepath READ getSearchByFilepath WRITE setSearchByFilepath NOTIFY searchByFilepathChanged)
        Q_PROPERTY(double scrollSpeedScale READ getScrollSpeedScale WRITE setScrollSpeedScale NOTIFY scrollSpeedScaleChanged)
        Q_PROPERTY(bool useSpellCheck READ getUseSpellCheck WRITE setUseSpellCheck NOTIFY useSpellCheckChanged)
        Q_PROPERTY(bool detectDuplicates READ getDetectDuplicates WRITE setDetectDuplicates NOTIFY detectDuplicatesChanged)
        Q_PROPERTY(bool userStatistics READ getUserStatistics WRITE setUserStatistics NOTIFY userStatisticsChanged)
        Q_PROPERTY(bool checkForUpdates READ getCheckForUpdates WRITE setCheckForUpdates NOTIFY checkForUpdatesChanged)
        Q_PROPERTY(bool autoDownloadUpdates READ getAutoDownloadUpdates WRITE setAutoDownloadUpdates NOTIFY autoDownloadUpdatesChanged)
        Q_PROPERTY(bool autoFindVectors READ getAutoFindVectors WRITE setAutoFindVectors NOTIFY autoFindVectorsChanged)
        Q_PROPERTY(QString selectedLocale READ getSelectedLocale WRITE setSelectedLocale NOTIFY selectedLocaleChanged)
        Q_PROPERTY(int selectedThemeIndex READ getSelectedThemeIndex WRITE setSelectedThemeIndex NOTIFY selectedThemeIndexChanged)
        Q_PROPERTY(bool useKeywordsAutoComplete READ getUseKeywordsAutoComplete WRITE setUseKeywordsAutoComplete NOTIFY useKeywordsAutoCompleteChanged)
        Q_PROPERTY(bool usePresetsAutoComplete READ getUsePresetsAutoComplete WRITE setUsePresetsAutoComplete NOTIFY usePresetsAutoCompleteChanged)
        Q_PROPERTY(bool useExifTool READ getUseExifTool WRITE setUseExifTool NOTIFY useExifToolChanged)
        Q_PROPERTY(bool useProxy READ getUseProxy WRITE setUseProxy NOTIFY useProxyChanged)
        Q_PROPERTY(QString proxyAddress READ getProxyAddress NOTIFY proxyAddressChanged)
        Q_PROPERTY(QString proxyUser READ getProxyUser NOTIFY proxyUserChanged)
        Q_PROPERTY(QString proxyPassword READ getProxyPassword NOTIFY proxyPasswordChanged)
        Q_PROPERTY(QString proxyPort READ getProxyPort NOTIFY proxyPortChanged)
        Q_PROPERTY(bool autoCacheImages READ getAutoCacheImages WRITE setAutoCacheImages NOTIFY autoCacheImagesChanged)
        Q_PROPERTY(bool verboseUpload READ getVerboseUpload WRITE setVerboseUpload NOTIFY verboseUploadChanged)
        Q_PROPERTY(bool useProgressiveSuggestionPreviews READ getUseProgressiveSuggestionPreviews WRITE setUseProgressiveSuggestionPreviews NOTIFY useProgressiveSuggestionPreviewsChanged)
        Q_PROPERTY(int progressiveSuggestionIncrement READ getProgressiveSuggestionIncrement WRITE setProgressiveSuggestionIncrement NOTIFY progressiveSuggestionIncrementChanged)
        Q_PROPERTY(bool useAutoImport READ getUseAutoImport WRITE setUseAutoImport NOTIFY useAutoImportChanged)

        Q_PROPERTY(QString appVersion READ getAppVersion CONSTANT)

        Q_PROPERTY(QString whatsNewText READ getWhatsNewText CONSTANT)
        Q_PROPERTY(QString termsAndConditionsText READ getTermsAndConditionsText CONSTANT)

    public:
        explicit SettingsModel(Common::ISystemEnvironment &environment, QObject *parent = 0);
        virtual ~SettingsModel() { }

    public:
        void saveLocale();
        void initializeConfigs();
        void syncronizeSettings() { sync(); }
        void migrateSettings();
        void clearLegacyUploadInfos();

    private:
        void moveSetting(QSettings &settingsQSettings, const QString &oldKey, const char* newKey, int type);
        void moveProxyHostSetting(QSettings &settingsQSettings);
        void wipeOldSettings(QSettings &settingsQSettings);
        void moveSettingsFromQSettingsToJson();

    public:
        ProxySettings *retrieveProxySettings();
        bool getIsTelemetryEnabled() const { return m_SettingsMap->boolValue(Constants::userStatistics, true); }
        int getSettingsVersion() const { return m_SettingsMap->intValue(Constants::settingsVersion); }

    public:
        Q_INVOKABLE void resetAllValues();
        Q_INVOKABLE void saveAllValues();
        Q_INVOKABLE void clearMasterPasswordSettings();
        Q_INVOKABLE void resetExifTool();
        Q_INVOKABLE void setExifTool(const QUrl &pathUrl);
        Q_INVOKABLE void retrieveAllValues();
        Q_INVOKABLE void raiseMasterPasswordSignal() { emit mustUseMasterPasswordChanged(m_MustUseMasterPassword); }
        Q_INVOKABLE void saveProxySetting(const QString &address, const QString &user, const QString &password, const QString &port);
        Q_INVOKABLE void updateSaveSession(bool value);

    private:
        void doReadAllValues();
        void doMoveSettingsFromQSettingsToJson();
        void doResetToDefault();
        void doSaveAllValues();
        void afterSaveHandler();
        void resetChangeStates();

    private:
        QString getAppVersion() const { return QCoreApplication::applicationVersion(); }
        QString getWhatsNewText() const;
        QString getTermsAndConditionsText() const;

    public:
        QString getUserAgentId() const { return m_State.getString(Constants::userAgentId); }
        QString getLegacyUploadHosts() const { return m_SettingsMap->stringValue(Constants::legacyUploadHosts); }
        QString getMasterPasswordHash() const { return m_SettingsMap->stringValue(Constants::masterPasswordHash); }
        bool getMustUseConfirmationDialogs() const { return m_SettingsMap->boolValue(Constants::useConfirmationDialogs, true); }

    public:
        Q_INVOKABLE bool needToShowWhatsNew();
        Q_INVOKABLE bool needToShowTextWhatsNew();
        Q_INVOKABLE void saveCurrentVersion();
        Q_INVOKABLE bool needToShowTermsAndConditions();
        Q_INVOKABLE void userAgreeHandler();
        Q_INVOKABLE void setUseMasterPassword(bool value);
        Q_INVOKABLE void setMasterPasswordHash();
        /*Q_INVOKABLE*/ void setUserAgentId(const QString &id);
        Q_INVOKABLE void onMasterPasswordSet();
        Q_INVOKABLE void onMasterPasswordUnset(bool firstTime);

    public:
        QString getExifToolPath() const { return m_ExifToolPath; }
        int getUploadTimeout() const { return m_UploadTimeout; }
        bool getMustUseMasterPassword() const { return m_MustUseMasterPassword; }
        bool getMustUseConfirmations() const { return m_MustUseConfirmations; }
        bool getSaveSession() const { return m_SaveSession; }
        bool getSaveBackups() const { return m_SaveBackups; }
        double getKeywordSizeScale() const { return m_KeywordSizeScale; }
        int getDismissDuration() const { return m_DismissDuration; }
        int getMaxParallelUploads() const { return m_MaxParallelUploads; }
        bool getFitSmallPreview() const { return m_FitSmallPreview; }
        bool getSearchUsingAnd() const { return m_SearchUsingAnd; }
        bool getSearchByFilepath() const { return m_SearchByFilepath; }
        double getScrollSpeedScale() const { return m_ScrollSpeedScale; }
        bool getUseSpellCheck() const { return m_UseSpellCheck; }
        bool getDetectDuplicates() const { return m_DetectDuplicates; }
        bool getUserStatistics() const { return m_UserStatistics; }
        bool getCheckForUpdates() const { return m_CheckForUpdates; }
        bool getAutoDownloadUpdates() const { return m_AutoDownloadUpdates; }
        bool getAutoFindVectors() const { return m_AutoFindVectors; }
        QString getSelectedLocale() const { return m_SelectedLocale; }
        int getSelectedThemeIndex() const { return m_SelectedThemeIndex; }
        bool getUseKeywordsAutoComplete() const { return m_UseKeywordsAutoComplete; }
        bool getUsePresetsAutoComplete() const { return m_UsePresetsAutoComplete; }
        bool getUseExifTool() const { return m_UseExifTool; }
        bool getUseProxy() const { return m_UseProxy; }
        QString getProxyAddress() const { return m_ProxySettings.m_Address; }
        QString getProxyUser() const { return m_ProxySettings.m_User; }
        QString getProxyPassword() const { return m_ProxySettings.m_Password; }
        QString getProxyPort() const { return m_ProxySettings.m_Port; }
        ProxySettings *getProxySettings() { return &m_ProxySettings; }
        bool getAutoCacheImages() const { return m_AutoCacheImages; }
        bool getVerboseUpload() const { return m_VerboseUpload; }
        bool getUseProgressiveSuggestionPreviews() const { return m_UseProgressiveSuggestionPreviews; }
        int getProgressiveSuggestionIncrement() const { return m_ProgressiveSuggestionIncrement; }
        int getUseDirectExiftoolExport() const { return m_UseDirectExiftoolExport; }
        bool getUseAutoImport() const { return m_UseAutoImport; }

    signals:
        void settingsReset();
        void settingsUpdated();
        void exifToolPathChanged(QString exifToolPath);
        void uploadTimeoutChanged(int uploadTimeout);
        void mustUseMasterPasswordChanged(bool mustUseMasterPassword);
        void mustUseConfirmationsChanged(bool mustUseConfirmations);
        void saveSessionChanged(bool saveSession);
        void saveBackupsChanged(bool saveBackups);
        void keywordSizeScaleChanged(double value);
        void dismissDurationChanged(int value);
        void maxParallelUploadsChanged(int value);
        void fitSmallPreviewChanged(bool value);
        void searchUsingAndChanged(bool value);
        void searchByFilepathChanged(bool value);
        void scrollSpeedScaleChanged(double value);
        void useSpellCheckChanged(bool value);
        void detectDuplicatesChanged(bool value);
        void userStatisticsChanged(bool value);
        void checkForUpdatesChanged(bool value);
        void autoDownloadUpdatesChanged(bool value);
        void autoFindVectorsChanged(bool value);
        void selectedLocaleChanged(QString value);
        void selectedThemeIndexChanged(int value);
        void useKeywordsAutoCompleteChanged(bool value);
        void usePresetsAutoCompleteChanged(bool value);
        void useExifToolChanged(bool value);
        void useProxyChanged(bool value);
        void proxyAddressChanged(QString value);
        void proxyUserChanged(QString value);
        void proxyPasswordChanged(QString value);
        void proxyPortChanged(QString value);
        void autoCacheImagesChanged(bool value);
        void verboseUploadChanged(bool verboseUpload);
        void useProgressiveSuggestionPreviewsChanged(bool progressiveSuggestionPreviews);
        void progressiveSuggestionIncrementChanged(int progressiveSuggestionIncrement);
        void useAutoImportChanged(bool value);

    public:
        void setExifToolPath(QString value);
        void setUploadTimeout(int uploadTimeout);
        void setMustUseMasterPassword(bool mustUseMasterPassword);
        void setMustUseConfirmations(bool mustUseConfirmations);
        void setSaveSession(bool saveSession);
        void setSaveBackups(bool saveBackups);
        void setKeywordSizeScale(double value);
        void setDismissDuration(int value);
        void setMaxParallelUploads(int value);
        void setFitSmallPreview(bool value);
        void setSearchUsingAnd(bool value);
        void setSearchByFilepath(bool value);
        void setScrollSpeedScale(double value);
        void setUseSpellCheck(bool value);
        void setDetectDuplicates(bool value);
        void setUserStatistics(bool value);
        void setCheckForUpdates(bool value);
        void setAutoDownloadUpdates(bool value);
        void setAutoFindVectors(bool value);
        void setSelectedLocale(QString value);
        void setSelectedThemeIndex(int value);
        void setUseKeywordsAutoComplete(bool value);
        void setUsePresetsAutoComplete(bool value);
        void setUseProxy(bool value);
        void setUseExifTool(bool value);
        void setAutoCacheImages(bool value);
        void setVerboseUpload(bool verboseUpload);
        void setUseProgressiveSuggestionPreviews(bool useProgressiveSuggestionPreviews);
        void setProgressiveSuggestionIncrement(int progressiveSuggestionIncrement);
        void setUseDirectExiftoolExport(bool value);
        void setUseAutoImport(bool value);

    public slots:
        void onRecommendedExiftoolFound(const QString &path);

#ifndef INTEGRATION_TESTS
    private:
#else
    public:
#endif
        void resetToDefault();

    private:
        void resetProxySetting();

    private:
        void sync();
        QString serializeProxyForSettings(ProxySettings &settings);
        void deserializeProxyFromSettings(const QString &serialized);

        // DelayedActionEntity implementation
    protected:
        virtual void doKillTimer(int timerId) override { this->killTimer(timerId); }
        virtual int doStartTimer(int interval, Qt::TimerType timerType) override { return this->startTimer(interval, timerType); }
        virtual void doOnTimer() override { doSaveAllValues(); }
        virtual void timerEvent(QTimerEvent *event) override { onQtTimer(event); }
        virtual void callBaseTimer(QTimerEvent *event) override { QObject::timerEvent(event); }

    private:
        Common::ISystemEnvironment &m_Environment;
        Common::StatefulEntity m_State;
        QMutex m_SettingsMutex;
        Helpers::LocalConfig m_Config;
        std::shared_ptr<Helpers::JsonObjectMap> m_SettingsMap;
        std::shared_ptr<Helpers::JsonObjectMap> m_ExperimentalMap;
        QString m_ExifToolPath;
        QString m_SelectedLocale;
        double m_KeywordSizeScale;
        double m_ScrollSpeedScale;
        int m_UploadTimeout; // in seconds
        int m_DismissDuration;
        int m_MaxParallelUploads;
        int m_SelectedThemeIndex;
        bool m_MustUseMasterPassword;
        bool m_MustUseConfirmations;
        bool m_SaveSession;
        bool m_SaveBackups;
        bool m_FitSmallPreview;
        bool m_SearchUsingAnd;
        bool m_SearchByFilepath;
        bool m_UseSpellCheck;
        bool m_DetectDuplicates;
        bool m_UserStatistics;
        bool m_CheckForUpdates;
        bool m_AutoDownloadUpdates;
        bool m_DictsPathChanged;
        bool m_UseSpellCheckChanged;
        bool m_DetectDuplicatesChanged;
        bool m_AutoFindVectors;
        bool m_UseKeywordsAutoComplete;
        bool m_UsePresetsAutoComplete;
        bool m_UseExifTool;
        bool m_UseProxy;
        ProxySettings m_ProxySettings;
        bool m_AutoCacheImages;
        bool m_VerboseUpload;
        bool m_UseProgressiveSuggestionPreviews;
        int m_ProgressiveSuggestionIncrement;
        bool m_UseDirectExiftoolExport;
        bool m_UseAutoImport;
        bool m_ExiftoolPathChanged;
    };
}

#endif // SETTINGSMODEL_H

/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "settingsmodel.h"
#include <QQmlEngine>
#include "../Common/defines.h"
#include "../Models/artitemsmodel.h"
#include "../Maintenance/maintenanceservice.h"
#include "../MetadataIO/metadataiocoordinator.h"
#include "../Commands/commandmanager.h"
#include "filteredartitemsproxymodel.h"

#ifdef Q_OS_MAC
#  define DEFAULT_EXIFTOOL "/usr/bin/exiftool"
#elif APPVEYOR
#  define DEFAULT_EXIFTOOL "c:/projects/xpiks-deps/windows-3rd-party-bin/exiftool.exe"
#else
#  define DEFAULT_EXIFTOOL "exiftool"
#endif

#ifdef QT_DEBUG
    #ifdef INTEGRATION_TESTS
        #define SETTINGS_FILE "integration_settings.json"
    #else
        #define SETTINGS_FILE "debug_settings.json"
    #endif
#else
#define SETTINGS_FILE "settings.json"
#endif

#define EXPERIMENTAL_KEY QLatin1String("experimental")

#define CURRENT_SETTINGS_VERSION 1

#define DEFAULT_USE_MASTERPASSWORD false
#define DEFAULT_UPLOAD_TIMEOUT 10
#define DEFAULT_USE_CONFIRMATIONS false
#define DEFAULT_SAVE_SESSION false
#define DEFAULT_SAVE_BACKUPS true
#define DEFAULT_KEYWORD_SIZE_SCALE 1.0
#define DEFAULT_DISMISS_DURATION 10
#define DEFAULT_MAX_PARALLEL_UPLOADS 2
#define DEFAULT_FIT_SMALL_PREVIEW false
#define DEFAULT_SEARCH_USING_AND true
#define DEFAULT_SEARCH_BY_FILEPATH true
#define DEFAULT_SCROLL_SPEED_SCALE 1.0
#define DEFAULT_USE_SPELL_CHECK true
#define DEFAULT_HAVE_USER_CONSENT false
#define DEFAULT_COLLECT_USER_STATISTICS true
#define DEFAULT_CHECK_FOR_UPDATES true
#define DEFAULT_AUTO_DOWNLOAD_UPDATES true
#define DEFAULT_AUTO_FIND_VECTORS true
#define DEFAULT_LOCALE "en_US"
#define DEFAULT_SELECTED_THEME_INDEX 0
#define DEFAULT_USE_KEYWORDS_AUTO_COMPLETE true
#define DEFAULT_USE_PRESETS_AUTO_COMPLETE true
#define DEFAULT_USE_EXIFTOOL true
#define DEFAULT_USE_PROXY false
#define DEFAULT_PROXY_HOST ""
#define DEFAULT_USE_PROGRESSIVE_SUGGESTION_PREVIEWS false
#define DEFAULT_PROGRESSIVE_SUGGESTION_INCREMENT 10

#ifdef QT_NO_DEBUG
    #define DEFAULT_USE_AUTOIMPORT true
    #define DEFAULT_AUTO_CACHE_IMAGES true
    #define DEFAULT_VERBOSE_UPLOAD false
    #define DEFAULT_USE_DIRECT_EXIFTOOL_EXPORT false
    #define DEFAULT_DETECT_DUPLICATES false
#else
    #define DEFAULT_DETECT_DUPLICATES true

    #ifdef INTEGRATION_TESTS
        // used in INTEGRATION TESTS
        #define DEFAULT_USE_AUTOIMPORT false
        #define DEFAULT_USE_DIRECT_EXIFTOOL_EXPORT true
        #define DEFAULT_AUTO_CACHE_IMAGES false
        #define DEFAULT_VERBOSE_UPLOAD false
    #else
        #define DEFAULT_USE_AUTOIMPORT true
        #define DEFAULT_AUTO_CACHE_IMAGES true
        #define DEFAULT_VERBOSE_UPLOAD true
        #define DEFAULT_USE_DIRECT_EXIFTOOL_EXPORT false
    #endif
#endif

#define SETTINGS_SAVING_INTERVAL 3000
#define SETTINGS_DELAY_TIMES 5

namespace Models {
    int ensureInBounds(int value, int boundA, int boundB) {
        Q_ASSERT(boundA <= boundB);

        if (value < boundA) {
            value = boundA;
        }

        if (value > boundB) {
            value = boundB;
        }

        return value;
    }

    double ensureInBounds(double value, double boundA, double boundB) {
        Q_ASSERT(boundA <= boundB);

        // double nan
        if (value != value) {
            value = boundA;
        }

        if (value < boundA) {
            value = boundA;
        }

        if (value > boundB) {
            value = boundB;
        }

        return value;
    }

    void clearSetting(QSettings &oldSettings, const char *settingName) {
        oldSettings.remove(QLatin1String(settingName));
    }

    SettingsModel::SettingsModel(Common::ISystemEnvironment &environment, QObject *parent) :
        QObject(parent),
        Common::BaseEntity(),
        Common::DelayedActionEntity(SETTINGS_SAVING_INTERVAL, SETTINGS_DELAY_TIMES),
        m_Environment(environment),
        m_State("settings", environment),
        m_ExifToolPath(DEFAULT_EXIFTOOL),
        m_SelectedLocale(DEFAULT_LOCALE),
        m_KeywordSizeScale(DEFAULT_KEYWORD_SIZE_SCALE),
        m_ScrollSpeedScale(DEFAULT_SCROLL_SPEED_SCALE),
        m_UploadTimeout(DEFAULT_UPLOAD_TIMEOUT),
        m_DismissDuration(DEFAULT_DISMISS_DURATION),
        m_MaxParallelUploads(DEFAULT_MAX_PARALLEL_UPLOADS),
        m_SelectedThemeIndex(DEFAULT_SELECTED_THEME_INDEX),
        m_MustUseMasterPassword(DEFAULT_USE_MASTERPASSWORD),
        m_MustUseConfirmations(DEFAULT_USE_CONFIRMATIONS),
        m_SaveSession(DEFAULT_SAVE_SESSION),
        m_SaveBackups(DEFAULT_SAVE_BACKUPS),
        m_FitSmallPreview(DEFAULT_FIT_SMALL_PREVIEW),
        m_SearchUsingAnd(DEFAULT_SEARCH_USING_AND),
        m_SearchByFilepath(DEFAULT_SEARCH_BY_FILEPATH),
        m_UseSpellCheck(DEFAULT_USE_SPELL_CHECK),
        m_DetectDuplicates(DEFAULT_DETECT_DUPLICATES),
        m_UserStatistics(DEFAULT_COLLECT_USER_STATISTICS),
        m_CheckForUpdates(DEFAULT_CHECK_FOR_UPDATES),
        m_AutoDownloadUpdates(DEFAULT_AUTO_DOWNLOAD_UPDATES),
        m_DictsPathChanged(false),
        m_UseSpellCheckChanged(false),
        m_DetectDuplicatesChanged(false),
        m_AutoFindVectors(DEFAULT_AUTO_FIND_VECTORS),
        m_UseKeywordsAutoComplete(DEFAULT_USE_KEYWORDS_AUTO_COMPLETE),
        m_UsePresetsAutoComplete(DEFAULT_USE_PRESETS_AUTO_COMPLETE),
        m_UseExifTool(DEFAULT_USE_EXIFTOOL),
        m_UseProxy(DEFAULT_USE_PROXY),
        m_AutoCacheImages(DEFAULT_AUTO_CACHE_IMAGES),
        m_VerboseUpload(DEFAULT_VERBOSE_UPLOAD),
        m_UseProgressiveSuggestionPreviews(DEFAULT_USE_PROGRESSIVE_SUGGESTION_PREVIEWS),
        m_ProgressiveSuggestionIncrement(DEFAULT_PROGRESSIVE_SUGGESTION_INCREMENT),
        m_UseDirectExiftoolExport(DEFAULT_USE_DIRECT_EXIFTOOL_EXPORT),
        m_UseAutoImport(DEFAULT_USE_AUTOIMPORT),
        m_ExiftoolPathChanged(false)
    {
    }

    void SettingsModel::saveLocale() {
        LOG_DEBUG << "#";
        QMutexLocker locker(&m_SettingsMutex);
        Q_UNUSED(locker);
        setValue(Constants::selectedLocale, m_SelectedLocale);
        sync();
    }

    void SettingsModel::initializeConfigs() {
        LOG_DEBUG << "#";

        QString localConfigPath = m_Environment.filepath(SETTINGS_FILE);
        m_Config.initConfig(localConfigPath);
        m_State.init();

        QJsonDocument &doc = m_Config.getConfig();
        if (doc.isObject()) {
            m_SettingsJson = doc.object();

            if (m_SettingsJson.contains(EXPERIMENTAL_KEY)) {
                QJsonValue experimental = m_SettingsJson[EXPERIMENTAL_KEY];
                if (experimental.isObject()) {
                    m_ExperimentalJson = experimental.toObject();
                }
            }
        } else {
            LOG_WARNING << "JSON document doesn't contain an object";
        }
    }

    void SettingsModel::migrateSettings() {
        const int settingsVersion = getSettingsVersion();
        LOG_INFO << "Current settings version:" << settingsVersion;

        if (settingsVersion < CURRENT_SETTINGS_VERSION) {
            moveSettingsFromQSettingsToJson();
        }
    }

    void SettingsModel::clearLegacyUploadInfos() {
        LOG_DEBUG << "#";

        if (containsValue(Constants::legacyUploadHosts)) {
            LOG_INFO << "Removing legacy upload hosts info";
            QString empty = "";
            setValue(Constants::legacyUploadHosts, empty);
            deleteValue(Constants::legacyUploadHosts);
            sync();
        }
    }

    void SettingsModel::moveSetting(QSettings &oldSettings, const QString &oldKey, const char *newKey, int type) {
        if (!oldSettings.contains(oldKey)) {
            LOG_WARNING << oldKey << "key is missing";
            return;
        }

        auto oldValue = oldSettings.value(oldKey);
        if (oldValue.convert(type)) {
            auto newValue = QJsonValue::fromVariant(oldValue);
            setValue(newKey, newValue);
        } else {
            LOG_WARNING <<"Failed to convert value in key" << oldKey;
        }
    }

    void SettingsModel::moveProxyHostSetting(QSettings &oldSettings) {
        QString oldKey = QLatin1String(Constants::PROXY_HOST);

        if (!oldSettings.contains(oldKey)) {
            LOG_WARNING << "PROXY_HOST key is missing";
            return;
        }

        QVariant qvalue = oldSettings.value(oldKey);
        ProxySettings proxySettings = qvalue.value<ProxySettings>();
        setValue(Constants::proxyHost, serializeProxyForSettings(proxySettings));
    }

    void SettingsModel::wipeOldSettings(QSettings &oldSettings) {
        using namespace Constants;

        clearSetting(oldSettings, PATH_TO_EXIFTOOL);
        clearSetting(oldSettings, SAVE_BACKUPS);
        clearSetting(oldSettings, KEYWORD_SIZE_SCALE);
        clearSetting(oldSettings, DISMISS_DURATION);
        clearSetting(oldSettings, FIT_SMALL_PREVIEW);
        clearSetting(oldSettings, SEARCH_USING_AND);
        clearSetting(oldSettings, SEARCH_BY_FILEPATH);
        clearSetting(oldSettings, DICT_PATH);
        clearSetting(oldSettings, USER_STATISTICS);
        clearSetting(oldSettings, CHECK_FOR_UPDATES);
        clearSetting(oldSettings, NUMBER_OF_LAUNCHES);
        clearSetting(oldSettings, APP_WINDOW_WIDTH);
        clearSetting(oldSettings, APP_WINDOW_HEIGHT);
        clearSetting(oldSettings, APP_WINDOW_X);
        clearSetting(oldSettings, APP_WINDOW_Y);
        clearSetting(oldSettings, AUTO_FIND_VECTORS);
        clearSetting(oldSettings, USE_PROXY);
        clearSetting(oldSettings, PROXY_HOST);
        clearSetting(oldSettings, UPLOAD_HOSTS);
        clearSetting(oldSettings, USE_MASTER_PASSWORD);
        clearSetting(oldSettings, MASTER_PASSWORD_HASH);
        clearSetting(oldSettings, ONE_UPLOAD_SECONDS_TIMEMOUT);
        clearSetting(oldSettings, USE_CONFIRMATION_DIALOGS);
        clearSetting(oldSettings, RECENT_DIRECTORIES);
        clearSetting(oldSettings, RECENT_FILES);
        clearSetting(oldSettings, MAX_PARALLEL_UPLOADS);
        clearSetting(oldSettings, USE_SPELL_CHECK);
        clearSetting(oldSettings, USER_AGENT_ID);
        clearSetting(oldSettings, INSTALLED_VERSION);
        clearSetting(oldSettings, USER_CONSENT);
        clearSetting(oldSettings, SELECTED_LOCALE);
        clearSetting(oldSettings, SELECTED_THEME_INDEX);
        clearSetting(oldSettings, USE_AUTO_COMPLETE);
        clearSetting(oldSettings, USE_EXIFTOOL);
        clearSetting(oldSettings, CACHE_IMAGES_AUTOMATICALLY);
        clearSetting(oldSettings, SCROLL_SPEED_SENSIVITY);
        clearSetting(oldSettings, AUTO_DOWNLOAD_UPDATES);
        clearSetting(oldSettings, AVAILABLE_UPDATE_VERSION);
        clearSetting(oldSettings, ARTWORK_EDIT_RIGHT_PANE_WIDTH);
        clearSetting(oldSettings, TRANSLATOR_SELECTED_DICT_INDEX);
    }

    void SettingsModel::moveSettingsFromQSettingsToJson() {
        LOG_DEBUG << "#";

        {
            QMutexLocker locker(&m_SettingsMutex);
            Q_UNUSED(locker);

            doMoveSettingsFromQSettingsToJson();
        }

        Encryption::SecretsManager *secretsManager = m_CommandManager->getSecretsManager();
        Models::UploadInfoRepository *uploadInfoRepository = m_CommandManager->getUploadInfoRepository();
        secretsManager->setMasterPasswordHash(getMasterPasswordHash());
        uploadInfoRepository->initFromString(getLegacyUploadHosts());
    }

    ProxySettings *SettingsModel::retrieveProxySettings() {
        ProxySettings *result = nullptr;

        if (getUseProxy()) {
            result = &m_ProxySettings;
        }

        return result;
    }

    void SettingsModel::resetAllValues() {
        LOG_DEBUG << "#";

        {
            QMutexLocker locker(&m_SettingsMutex);
            Q_UNUSED(locker);

            doResetToDefault();
            doSaveAllValues();
        }

        emit settingsReset();
        afterSaveHandler();
    }

    void SettingsModel::saveAllValues() {
        LOG_INFO << "#";

        {
            QMutexLocker locker(&m_SettingsMutex);
            Q_UNUSED(locker);
            doSaveAllValues();
        }

        afterSaveHandler();
    }

    void SettingsModel::clearMasterPasswordSettings() {
        setMustUseMasterPassword(false);
        setValue(Constants::masterPasswordHash, "");
        setValue(Constants::useMasterPassword, false);
        sync();
    }

    void SettingsModel::resetExifTool() {
        LOG_DEBUG<< "#";
        setExifToolPath(DEFAULT_EXIFTOOL);
    }

    void SettingsModel::setExifTool(const QUrl &pathUrl) {
        LOG_DEBUG << pathUrl;
        setExifToolPath(pathUrl.toLocalFile());
    }

    void SettingsModel::retrieveAllValues() {
        LOG_INFO << "#";

        QMutexLocker locker(&m_SettingsMutex);
        Q_UNUSED(locker);

        doReadAllValues();
    }

    void SettingsModel::saveProxySetting(const QString &address, const QString &user,
                                         const QString &password, const QString &port) {
        QString tAddress = address.trimmed();
        QString tUser = user.trimmed();
        QString tPassword = password.trimmed();
        QString tPort = port.trimmed();

        if (tAddress != m_ProxySettings.m_Address) {
            m_ProxySettings.m_Address = tAddress;
            emit proxyAddressChanged(tAddress);
        }

        if (tUser != m_ProxySettings.m_User) {
            m_ProxySettings.m_User = tUser;
            emit proxyUserChanged(tUser);
        }

        if (tPassword != m_ProxySettings.m_Password) {
            m_ProxySettings.m_Password = tPassword;
            emit proxyPasswordChanged(tPassword);
        }

        if (tPort != m_ProxySettings.m_Port) {
            m_ProxySettings.m_Port = tPort;
            emit proxyPortChanged(tPort);
        }
    }

    void SettingsModel::updateSaveSession(bool value) {
        LOG_DEBUG << value;

        if (m_SaveSession == value) {
            return;
        }

        m_SaveSession = value;
        emit saveSessionChanged(value);

        setValue(Constants::saveSession, value);
        sync();

        if (value) {
            xpiks()->saveSessionInBackground();
        }
    }

    void SettingsModel::doReadAllValues() {
        using namespace Constants;

        setExifToolPath(stringValue(pathToExifTool, DEFAULT_EXIFTOOL));
        setUploadTimeout(intValue(oneUploadSecondsTimeout, DEFAULT_UPLOAD_TIMEOUT));
        setMustUseMasterPassword(boolValue(useMasterPassword, DEFAULT_USE_MASTERPASSWORD));
        setMustUseConfirmations(boolValue(useConfirmationDialogs, DEFAULT_USE_CONFIRMATIONS));
        setSaveSession(boolValue(saveSession, DEFAULT_SAVE_SESSION));
        setSaveBackups(boolValue(saveBackups, DEFAULT_SAVE_BACKUPS));
        setKeywordSizeScale(doubleValue(keywordSizeScale, DEFAULT_KEYWORD_SIZE_SCALE));
        setDismissDuration(intValue(dismissDuration, DEFAULT_DISMISS_DURATION));
        setMaxParallelUploads(intValue(maxParallelUploads, DEFAULT_MAX_PARALLEL_UPLOADS));
        setFitSmallPreview(boolValue(fitSmallPreview, DEFAULT_FIT_SMALL_PREVIEW));
        setSearchUsingAnd(boolValue(searchUsingAnd, DEFAULT_SEARCH_USING_AND));
        setSearchByFilepath(boolValue(searchByFilepath, DEFAULT_SEARCH_BY_FILEPATH));
        setScrollSpeedScale(doubleValue(scrollSpeedSensivity, DEFAULT_SCROLL_SPEED_SCALE));
        setUseSpellCheck(boolValue(useSpellCheck, DEFAULT_USE_SPELL_CHECK));
        setDetectDuplicates(boolValue(detectDuplicates, DEFAULT_DETECT_DUPLICATES));
        setUserStatistics(boolValue(userStatistics, DEFAULT_COLLECT_USER_STATISTICS));
        setCheckForUpdates(boolValue(checkForUpdates, DEFAULT_CHECK_FOR_UPDATES));
        setAutoDownloadUpdates(boolValue(autoDownloadUpdates, DEFAULT_AUTO_DOWNLOAD_UPDATES));
        setAutoFindVectors(boolValue(autoFindVectors, DEFAULT_AUTO_FIND_VECTORS));
        setSelectedLocale(stringValue(selectedLocale, DEFAULT_LOCALE));
        setSelectedThemeIndex(intValue(selectedThemeIndex, DEFAULT_SELECTED_THEME_INDEX));
        setUseKeywordsAutoComplete(boolValue(useKeywordsAutoComplete, DEFAULT_USE_KEYWORDS_AUTO_COMPLETE));
        setUsePresetsAutoComplete(boolValue(usePresetsAutoComplete, DEFAULT_USE_PRESETS_AUTO_COMPLETE));
        setUseExifTool(boolValue(useExifTool, DEFAULT_USE_EXIFTOOL));
        setUseProxy(boolValue(useProxy, DEFAULT_USE_PROXY));
        setVerboseUpload(boolValue(verboseUpload, DEFAULT_VERBOSE_UPLOAD));

        setUseProgressiveSuggestionPreviews(expBoolValue(useProgressiveSuggestionPreviews, DEFAULT_USE_PROGRESSIVE_SUGGESTION_PREVIEWS));
        setProgressiveSuggestionIncrement(expIntValue(progressiveSuggestionIncrement, DEFAULT_PROGRESSIVE_SUGGESTION_INCREMENT));
        setUseDirectExiftoolExport(expBoolValue(useDirectExiftoolExport, DEFAULT_USE_DIRECT_EXIFTOOL_EXPORT));
        setUseAutoImport(expBoolValue(useAutoImport, DEFAULT_USE_AUTOIMPORT));

        deserializeProxyFromSettings(stringValue(proxyHost, DEFAULT_PROXY_HOST));

        setAutoCacheImages(boolValue(cacheImagesAutomatically, DEFAULT_AUTO_CACHE_IMAGES));

        resetChangeStates();
    }

    void SettingsModel::doMoveSettingsFromQSettingsToJson() {
        LOG_DEBUG << "#";

        using namespace Constants;

        QSettings oldSettings(QSettings::UserScope,
                              QCoreApplication::instance()->organizationName(),
                              QCoreApplication::instance()->applicationName());

        moveSetting(oldSettings, PATH_TO_EXIFTOOL, pathToExifTool, QMetaType::QString);
        moveSetting(oldSettings, SAVE_BACKUPS, saveBackups, QMetaType::Bool);
        moveSetting(oldSettings, KEYWORD_SIZE_SCALE, keywordSizeScale, QMetaType::Double);
        moveSetting(oldSettings, DISMISS_DURATION, dismissDuration, QMetaType::Int);
        moveSetting(oldSettings, FIT_SMALL_PREVIEW, fitSmallPreview, QMetaType::Bool);
        moveSetting(oldSettings, SEARCH_USING_AND, searchUsingAnd, QMetaType::Bool);
        moveSetting(oldSettings, SEARCH_BY_FILEPATH, searchByFilepath, QMetaType::Bool);
        //moveSetting(oldSettings, DICT_PATH, dictPath, QMetaType::QString);
        moveSetting(oldSettings, USER_STATISTICS, userStatistics, QMetaType::Bool);
        moveSetting(oldSettings, CHECK_FOR_UPDATES, checkForUpdates, QMetaType::Bool);
        //moveSetting(oldSettings, NUMBER_OF_LAUNCHES, numberOfLaunches, QMetaType::Int);
        //moveSetting(oldSettings, APP_WINDOW_WIDTH, appWindowWidth, QMetaType::Int);
        //moveSetting(oldSettings, APP_WINDOW_HEIGHT, appWindowHeight, QMetaType::Int);
        //moveSetting(oldSettings, APP_WINDOW_X, appWindowX, QMetaType::Int);
        //moveSetting(oldSettings, APP_WINDOW_Y, appWindowY, QMetaType::Int);
        moveSetting(oldSettings, AUTO_FIND_VECTORS, autoFindVectors, QMetaType::Bool);
        moveSetting(oldSettings, USE_PROXY, useProxy, QMetaType::Bool);
        moveProxyHostSetting(oldSettings);
        moveSetting(oldSettings, UPLOAD_HOSTS, legacyUploadHosts, QMetaType::QString);
        moveSetting(oldSettings, USE_MASTER_PASSWORD, useMasterPassword, QMetaType::Bool);
        moveSetting(oldSettings, MASTER_PASSWORD_HASH, masterPasswordHash, QMetaType::QString);
        moveSetting(oldSettings, ONE_UPLOAD_SECONDS_TIMEMOUT, oneUploadSecondsTimeout, QMetaType::Int);
        moveSetting(oldSettings, USE_CONFIRMATION_DIALOGS, useConfirmationDialogs, QMetaType::Bool);
        //moveSetting(oldSettings, RECENT_DIRECTORIES, recentDirectories, QMetaType::QString);
        //moveSetting(oldSettings, RECENT_FILES, recentFiles, QMetaType::QString);
        moveSetting(oldSettings, MAX_PARALLEL_UPLOADS, maxParallelUploads, QMetaType::Int);
        moveSetting(oldSettings, USE_SPELL_CHECK, useSpellCheck, QMetaType::Bool);
        //moveSetting(oldSettings, USER_AGENT_ID, userAgentId, QMetaType::QString);
        //moveSetting(oldSettings, INSTALLED_VERSION, installedVersion, QMetaType::Int);
        //moveSetting(oldSettings, USER_CONSENT, userConsent, QMetaType::Bool);
        moveSetting(oldSettings, SELECTED_LOCALE, selectedLocale, QMetaType::QString);
        moveSetting(oldSettings, SELECTED_THEME_INDEX, selectedThemeIndex, QMetaType::Int);
        moveSetting(oldSettings, USE_AUTO_COMPLETE, useKeywordsAutoComplete, QMetaType::Bool);
        moveSetting(oldSettings, USE_EXIFTOOL, useExifTool, QMetaType::Bool);
        moveSetting(oldSettings, CACHE_IMAGES_AUTOMATICALLY, cacheImagesAutomatically, QMetaType::Bool);
        moveSetting(oldSettings, SCROLL_SPEED_SENSIVITY, scrollSpeedSensivity, QMetaType::Double);
        moveSetting(oldSettings, AUTO_DOWNLOAD_UPDATES, autoDownloadUpdates, QMetaType::Bool);
        moveSetting(oldSettings, PATH_TO_UPDATE, pathToUpdate, QMetaType::QString);
        moveSetting(oldSettings, AVAILABLE_UPDATE_VERSION, availableUpdateVersion, QMetaType::Int);
        moveSetting(oldSettings, ARTWORK_EDIT_RIGHT_PANE_WIDTH, artworkEditRightPaneWidth, QMetaType::Int);
        //moveSetting(oldSettings, TRANSLATOR_SELECTED_DICT_INDEX, translatorSelectedDictIndex, QMetaType::Int);

        // apply imported settings
        doReadAllValues();

        setValue(settingsVersion, CURRENT_SETTINGS_VERSION);
        sync();

        wipeOldSettings(oldSettings);
    }

    void SettingsModel::doResetToDefault() {
        LOG_DEBUG << "#";

        setExifToolPath(DEFAULT_EXIFTOOL);
        setUploadTimeout(DEFAULT_UPLOAD_TIMEOUT);
        setMustUseMasterPassword(DEFAULT_USE_MASTERPASSWORD);
        setMustUseConfirmations(DEFAULT_USE_CONFIRMATIONS);
        setSaveSession(DEFAULT_SAVE_SESSION);
        setSaveBackups(DEFAULT_SAVE_BACKUPS);
        setKeywordSizeScale(DEFAULT_KEYWORD_SIZE_SCALE);
        setDismissDuration(DEFAULT_DISMISS_DURATION);
        setMaxParallelUploads(DEFAULT_MAX_PARALLEL_UPLOADS);
        setFitSmallPreview(DEFAULT_FIT_SMALL_PREVIEW);
        setSearchUsingAnd(DEFAULT_SEARCH_USING_AND);
        setSearchByFilepath(DEFAULT_SEARCH_BY_FILEPATH);
        setScrollSpeedScale(DEFAULT_SCROLL_SPEED_SCALE);
        setUseSpellCheck(DEFAULT_USE_SPELL_CHECK);
        setDetectDuplicates(DEFAULT_DETECT_DUPLICATES);
        setUserStatistics(DEFAULT_COLLECT_USER_STATISTICS);
        setCheckForUpdates(DEFAULT_CHECK_FOR_UPDATES);
        setAutoDownloadUpdates(DEFAULT_AUTO_DOWNLOAD_UPDATES);
        setAutoFindVectors(DEFAULT_AUTO_FIND_VECTORS);
        setSelectedLocale(DEFAULT_LOCALE);
        setSelectedThemeIndex(DEFAULT_SELECTED_THEME_INDEX);
        setUseKeywordsAutoComplete(DEFAULT_USE_KEYWORDS_AUTO_COMPLETE);
        setUsePresetsAutoComplete(DEFAULT_USE_PRESETS_AUTO_COMPLETE);
        setUseExifTool(DEFAULT_USE_EXIFTOOL);
        setUseProxy(DEFAULT_USE_PROXY);
        resetProxySetting();
        setAutoCacheImages(DEFAULT_AUTO_CACHE_IMAGES);
        setVerboseUpload(DEFAULT_VERBOSE_UPLOAD);

        setUseProgressiveSuggestionPreviews(DEFAULT_USE_PROGRESSIVE_SUGGESTION_PREVIEWS);
        setProgressiveSuggestionIncrement(DEFAULT_PROGRESSIVE_SUGGESTION_INCREMENT);
        setUseDirectExiftoolExport(DEFAULT_USE_DIRECT_EXIFTOOL_EXPORT);
        setUseAutoImport(DEFAULT_USE_AUTOIMPORT);

        justChanged();

        resetChangeStates();
    }

    void SettingsModel::doSaveAllValues() {
        LOG_DEBUG << "#";
        using namespace Constants;

        setValue(pathToExifTool, m_ExifToolPath);
        setValue(oneUploadSecondsTimeout, m_UploadTimeout);
        setValue(useMasterPassword, m_MustUseMasterPassword);
        setValue(useConfirmationDialogs, m_MustUseConfirmations);
        setValue(saveSession, m_SaveSession);
        setValue(saveBackups, m_SaveBackups);
        setValue(keywordSizeScale, m_KeywordSizeScale);
        setValue(dismissDuration, m_DismissDuration);
        setValue(maxParallelUploads, m_MaxParallelUploads);
        setValue(fitSmallPreview, m_FitSmallPreview);
        setValue(searchUsingAnd, m_SearchUsingAnd);
        setValue(searchByFilepath, m_SearchByFilepath);
        setValue(scrollSpeedSensivity, m_ScrollSpeedScale);
        setValue(useSpellCheck, m_UseSpellCheck);
        setValue(detectDuplicates, m_DetectDuplicates);
        setValue(userStatistics, m_UserStatistics);
        setValue(checkForUpdates, m_CheckForUpdates);
        setValue(autoDownloadUpdates, m_AutoDownloadUpdates);
        setValue(autoFindVectors, m_AutoFindVectors);
        setValue(selectedThemeIndex, m_SelectedThemeIndex);
        setValue(useKeywordsAutoComplete, m_UseKeywordsAutoComplete);
        setValue(usePresetsAutoComplete, m_UsePresetsAutoComplete);
        setValue(useExifTool, m_UseExifTool);
        setValue(useProxy, m_UseProxy);
        setValue(proxyHost, serializeProxyForSettings(m_ProxySettings));
        setValue(cacheImagesAutomatically, m_AutoCacheImages);
        setValue(verboseUpload, m_VerboseUpload);

        setExperimentalValue(useProgressiveSuggestionPreviews, m_UseProgressiveSuggestionPreviews);
        setExperimentalValue(progressiveSuggestionIncrement, m_ProgressiveSuggestionIncrement);
        setExperimentalValue(useDirectExiftoolExport, m_UseDirectExiftoolExport);
        setExperimentalValue(useAutoImport, m_UseAutoImport);

        if (!m_MustUseMasterPassword) {
            setValue(masterPasswordHash, "");
        }

        sync();
    }

    void SettingsModel::afterSaveHandler() {
        LOG_DEBUG << "#";

#if defined(Q_OS_LINUX)
        if (m_DictsPathChanged) {
            // TODO: check if need to restart depending on path
            xpiks()->restartSpellChecking();
            m_DictsPathChanged = false;
        }
#endif

        bool requiresSpellCheck = false;

        if (m_UseSpellCheckChanged) {
            if (!getUseSpellCheck()) {
                xpiks()->disableSpellChecking();
            } else {
                requiresSpellCheck = true;
            }
        }

        if (m_DetectDuplicatesChanged) {
            if (!getDetectDuplicates()) {
                xpiks()->disableDuplicatesCheck();
            } else {
                requiresSpellCheck = true;
            }
        }

        if (requiresSpellCheck) {
            auto *filteredModel = m_CommandManager->getFilteredArtItemsModel();
            filteredModel->spellCheckAllItems();
        }

        if (m_ExiftoolPathChanged) {
            xpiks()->autoDiscoverExiftool();
        }

        resetChangeStates();

        emit keywordSizeScaleChanged(m_KeywordSizeScale);
        emit settingsUpdated();
    }

    void SettingsModel::resetChangeStates() {
        m_DictsPathChanged = false;
        m_UseSpellCheckChanged = false;
        m_ExiftoolPathChanged = false;
        m_DetectDuplicatesChanged = false;
    }

    QString SettingsModel::getWhatsNewText() const {
        QString text;
        QString path;

#if !defined(Q_OS_LINUX)
        path = QCoreApplication::applicationDirPath();

#if defined(Q_OS_MAC)
        path += "/../Resources/";
#endif

        path += QDir::separator() + QLatin1String(Constants::WHATS_NEW_FILENAME);
        path = QDir::cleanPath(path);
#else
        // path inside appimage
        path = QCoreApplication::applicationDirPath() + QDir::separator() + Constants::WHATS_NEW_FILENAME;
#endif
        QFile file(QDir::cleanPath(path));
        if (file.open(QIODevice::ReadOnly)) {
            text = QString::fromUtf8(file.readAll());
            file.close();
        } else {
            LOG_WARNING << "whatsnew.txt file is not found on path" << path;

            path = QDir::current().absoluteFilePath(QLatin1String(Constants::WHATS_NEW_FILENAME));

            QFile currDirFile(path);
            if (currDirFile.open(QIODevice::ReadOnly)) {
                text = QString::fromUtf8(currDirFile.readAll());
                currDirFile.close();
            }
        }
        return text;
    }

    QString SettingsModel::getTermsAndConditionsText() const {
        QString text;
        QString path;

#if !defined(Q_OS_LINUX)
        path = QCoreApplication::applicationDirPath();

#if defined(Q_OS_MAC)
        path += "/../Resources/";
#endif

        path += QDir::separator() + QLatin1String(Constants::TERMS_AND_CONDITIONS_FILENAME);
        path = QDir::cleanPath(path);
#else
        // path inside appimage
        path = QCoreApplication::applicationDirPath() + QDir::separator() + Constants::TERMS_AND_CONDITIONS_FILENAME;
#endif
        QFile file(QDir::cleanPath(path));
        if (file.open(QIODevice::ReadOnly)) {
            text = QString::fromUtf8(file.readAll());
            file.close();
        } else {
            LOG_WARNING << "terms_and_conditions.txt file is not found on path" << path;

            path = QDir::current().absoluteFilePath(QLatin1String(Constants::TERMS_AND_CONDITIONS_FILENAME));

            QFile currDirFile(path);
            if (currDirFile.open(QIODevice::ReadOnly)) {
                text = QString::fromUtf8(currDirFile.readAll());
                currDirFile.close();
            }
        }

        return text;
    }

    bool SettingsModel::needToShowWhatsNew() {
        int lastVersion = m_State.getInt(Constants::installedVersion, 0);
        int installedMajorPart = lastVersion / XPIKS_VERSION_MAJOR_DIVISOR;
        int currentMajorPart = XPIKS_FULL_VERSION_INT / XPIKS_VERSION_MAJOR_DIVISOR;
        bool result = currentMajorPart > installedMajorPart;
        return result;
    }

    bool SettingsModel::needToShowTextWhatsNew() {
        int lastVersion = m_State.getInt(Constants::installedVersion, 0);
        int installedMajorPart = lastVersion / XPIKS_VERSION_MAJOR_DIVISOR;
        int currentMajorPart = XPIKS_FULL_VERSION_INT / XPIKS_VERSION_MAJOR_DIVISOR;
        bool result = (currentMajorPart == installedMajorPart) &&
                (XPIKS_FULL_VERSION_INT > lastVersion);
        return result;
    }

    void SettingsModel::saveCurrentVersion() {
        LOG_DEBUG << "version" << XPIKS_FULL_VERSION_INT;
        m_State.setValue(Constants::installedVersion, XPIKS_FULL_VERSION_INT);
        m_State.sync();
    }

    bool SettingsModel::needToShowTermsAndConditions() {
        bool haveConsent = m_State.getBool(Constants::userConsent, false);
        return !haveConsent;
    }

    void SettingsModel::setExifToolPath(QString value) {
        if (m_ExifToolPath == value)
            return;

        m_ExifToolPath = value;
        m_ExiftoolPathChanged = true;
        emit exifToolPathChanged(value);
        justChanged();
    }

    void SettingsModel::setUploadTimeout(int uploadTimeout) {
        if (m_UploadTimeout == uploadTimeout)
            return;

        m_UploadTimeout = ensureInBounds(uploadTimeout, 1, 300);
        emit uploadTimeoutChanged(m_UploadTimeout);
        m_ExiftoolPathChanged = true;
        justChanged();
    }

    void SettingsModel::setMustUseMasterPassword(bool mustUseMasterPassword) {
        if (m_MustUseMasterPassword == mustUseMasterPassword)
            return;

        m_MustUseMasterPassword = mustUseMasterPassword;
        emit mustUseMasterPasswordChanged(mustUseMasterPassword);
        justChanged();
    }

    void SettingsModel::setMustUseConfirmations(bool mustUseConfirmations) {
        if (m_MustUseConfirmations == mustUseConfirmations)
            return;

        m_MustUseConfirmations = mustUseConfirmations;
        emit mustUseConfirmationsChanged(mustUseConfirmations);
        justChanged();
    }

    void SettingsModel::setSaveSession(bool saveSession) {
        if (m_SaveSession == saveSession)
            return;

        m_SaveSession = saveSession;
        emit saveSessionChanged(saveSession);
        justChanged();
    }

    void SettingsModel::setSaveBackups(bool saveBackups) {
        if (m_SaveBackups == saveBackups)
            return;

        m_SaveBackups = saveBackups;
        emit saveBackupsChanged(saveBackups);
        justChanged();
    }

    void SettingsModel::setKeywordSizeScale(double value) {
        if (qAbs(m_KeywordSizeScale - value) <= SETTINGS_EPSILON)
            return;

        m_KeywordSizeScale = ensureInBounds(value, 1.0, 1.2);
        emit keywordSizeScaleChanged(m_KeywordSizeScale);
        justChanged();
    }

    void SettingsModel::setDismissDuration(int value) {
        if (m_DismissDuration == value)
            return;

        m_DismissDuration = ensureInBounds(value, 1, 100);
        emit dismissDurationChanged(m_DismissDuration);
        justChanged();
    }

    void SettingsModel::setMaxParallelUploads(int value) {
        if (m_MaxParallelUploads == value)
            return;

        m_MaxParallelUploads = ensureInBounds(value, 1, 4);
        emit maxParallelUploadsChanged(m_MaxParallelUploads);
        justChanged();
    }

    void SettingsModel::setFitSmallPreview(bool value) {
        if (m_FitSmallPreview == value)
            return;

        m_FitSmallPreview = value;
        emit fitSmallPreviewChanged(value);
        justChanged();
    }

    void SettingsModel::setSearchUsingAnd(bool value) {
        if (m_SearchUsingAnd == value)
            return;

        m_SearchUsingAnd = value;
        emit searchUsingAndChanged(value);
        justChanged();
    }

    void SettingsModel::setSearchByFilepath(bool value) {
        if (m_SearchByFilepath == value)
            return;

        m_SearchByFilepath = value;
        emit searchByFilepathChanged(value);
        justChanged();
    }

    void SettingsModel::setScrollSpeedScale(double value) {
        if (qAbs(m_ScrollSpeedScale - value) <= SETTINGS_EPSILON)
            return;

        m_ScrollSpeedScale = ensureInBounds(value, 0.1, 2.0);
        emit scrollSpeedScaleChanged(m_ScrollSpeedScale);
        justChanged();
    }

    void SettingsModel::setUseSpellCheck(bool value) {
        if (m_UseSpellCheck == value)
            return;

        m_UseSpellCheck = value;
        emit useSpellCheckChanged(value);
        m_UseSpellCheckChanged = true;
        justChanged();
    }

    void SettingsModel::setDetectDuplicates(bool value)  {
        if (m_DetectDuplicates == value)
            return;

        m_DetectDuplicates = value;
        m_DetectDuplicatesChanged = true;
        emit detectDuplicatesChanged(value);
        justChanged();
    }

    void SettingsModel::setUserStatistics(bool value) {
        if (m_UserStatistics == value)
            return;

        m_UserStatistics = value;
        emit userStatisticsChanged(value);
        justChanged();
    }

    void SettingsModel::setCheckForUpdates(bool value) {
        if (m_CheckForUpdates == value)
            return;

        m_CheckForUpdates = value;
        emit checkForUpdatesChanged(value);
        justChanged();
    }

    void SettingsModel::setAutoDownloadUpdates(bool value) {
        if (m_AutoDownloadUpdates == value)
            return;

        m_AutoDownloadUpdates = value;
        emit autoDownloadUpdatesChanged(value);
        justChanged();
    }

    void SettingsModel::setAutoFindVectors(bool value) {
        if (value != m_AutoFindVectors) {
            m_AutoFindVectors = value;
            emit autoFindVectorsChanged(value);
            justChanged();
        }
    }

    void SettingsModel::setSelectedLocale(QString value) {
        if (value != m_SelectedLocale) {
            m_SelectedLocale = value;
            emit selectedLocaleChanged(value);
            justChanged();
        }
    }

    void SettingsModel::setSelectedThemeIndex(int value) {
        if (value != m_SelectedThemeIndex) {
            m_SelectedThemeIndex = value;
            emit selectedThemeIndexChanged(value);
            justChanged();
        }
    }

    void SettingsModel::setUseKeywordsAutoComplete(bool value) {
        if (value != m_UseKeywordsAutoComplete) {
            m_UseKeywordsAutoComplete = value;
            emit useKeywordsAutoCompleteChanged(value);
            justChanged();
        }
    }

    void SettingsModel::setUsePresetsAutoComplete(bool value) {
        if (value != m_UsePresetsAutoComplete) {
            m_UsePresetsAutoComplete = value;
            emit usePresetsAutoCompleteChanged(value);
            justChanged();
        }
    }

    void SettingsModel::setUseProxy(bool value) {
        if (value != m_UseProxy) {
            m_UseProxy = value;
            emit useProxyChanged(value);
            justChanged();
        }
    }

    void SettingsModel::setUseExifTool(bool value) {
        if (value != m_UseExifTool) {
            m_UseExifTool = value;
            emit useExifToolChanged(value);
            justChanged();
        }
    }

    void SettingsModel::setAutoCacheImages(bool value) {
        if (value != m_AutoCacheImages) {
            m_AutoCacheImages = value;
            emit autoCacheImagesChanged(value);
            justChanged();
        }
    }

    void SettingsModel::setVerboseUpload(bool verboseUpload)
    {
        if (m_VerboseUpload == verboseUpload)
            return;

        m_VerboseUpload = verboseUpload;
        emit verboseUploadChanged(verboseUpload);
        justChanged();
    }

    void SettingsModel::setUseProgressiveSuggestionPreviews(bool useProgressiveSuggestionPreviews)
    {
        if (m_UseProgressiveSuggestionPreviews == useProgressiveSuggestionPreviews)
            return;

        m_UseProgressiveSuggestionPreviews = useProgressiveSuggestionPreviews;
        emit useProgressiveSuggestionPreviewsChanged(useProgressiveSuggestionPreviews);
        justChanged();
    }

    void SettingsModel::setProgressiveSuggestionIncrement(int progressiveSuggestionIncrement)
    {
        if (m_ProgressiveSuggestionIncrement == progressiveSuggestionIncrement)
            return;

        m_ProgressiveSuggestionIncrement = progressiveSuggestionIncrement;
        emit progressiveSuggestionIncrementChanged(progressiveSuggestionIncrement);
        justChanged();
    }

    void SettingsModel::setUseDirectExiftoolExport(bool value) {
        if (m_UseDirectExiftoolExport == value)
            return;

        m_UseDirectExiftoolExport = value;
        justChanged();
    }

    void SettingsModel::setUseAutoImport(bool value) {
        if (m_UseAutoImport == value)
            return;

        m_UseAutoImport = value;
        emit useAutoImportChanged(value);
        justChanged();
    }

    void SettingsModel::onRecommendedExiftoolFound(const QString &path) {
        LOG_INFO << path;
        QString existingExiftoolPath = getExifToolPath();

        if (existingExiftoolPath != path) {
            LOG_INFO << "Setting exiftool path to recommended";
            setExifToolPath(path);
        }
    }

    void SettingsModel::resetToDefault() {
        LOG_DEBUG << "#";
        QMutexLocker locker(&m_SettingsMutex);
        Q_UNUSED(locker);
        doResetToDefault();
    }

    void SettingsModel::resetProxySetting() {
        QString empty = "";
        SettingsModel::saveProxySetting(empty, empty, empty, empty);
    }

    void SettingsModel::protectHealthReporting() {
        const bool reportingEnabled = getUserStatistics();

        if (reportingEnabled) {
            m_State.setValue(Constants::numberOfLaunches, 0);
        } else {
            int numberOfLaunches = m_State.getInt(Constants::numberOfLaunches, 0);
            numberOfLaunches++;

            if (numberOfLaunches >= 31) {
                setUserStatistics(true);
                m_State.setValue(Constants::numberOfLaunches, 0);
                LOG_DEBUG << "Resetting health reporting to ON";
            } else {
                this->setValue(Constants::numberOfLaunches, numberOfLaunches);
                LOG_DEBUG << numberOfLaunches << "launches of Xpiks with Health reporting OFF";
            }
        }

        m_State.sync();
    }

    void SettingsModel::sync() {
        LOG_DEBUG << "Syncing settings";

        Helpers::LocalConfigDropper dropper(&m_Config);
        Q_UNUSED(dropper);

        m_SettingsJson[EXPERIMENTAL_KEY] = m_ExperimentalJson;

        QJsonDocument doc;
        doc.setObject(m_SettingsJson);

        m_Config.setConfig(doc);
        m_Config.saveToFile();
    }

    QString SettingsModel::serializeProxyForSettings(ProxySettings &settings) {
        QByteArray raw;
        QDataStream ds(&raw, QIODevice::WriteOnly);
        ds << settings;
        return QString::fromLatin1(raw.toBase64());
    }

    void SettingsModel::deserializeProxyFromSettings(const QString &serialized) {
        QByteArray originalData;
        originalData.append(serialized.toLatin1());
        QByteArray serializedBA = QByteArray::fromBase64(originalData);

        QDataStream ds(&serializedBA, QIODevice::ReadOnly);
        ds >> m_ProxySettings;
    }
}


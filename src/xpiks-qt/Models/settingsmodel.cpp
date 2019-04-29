/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "settingsmodel.h"

#include <QByteArray>
#include <QChar>
#include <QDataStream>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QIODevice>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonValueRef>
#include <QLatin1String>
#include <QMutexLocker>
#include <QSettings>
#include <QUrl>
#include <QVariant>
#include <QtDebug>
#include <QtGlobal>

#include "Common/delayedactionentity.h"
#include "Common/isystemenvironment.h"
#include "Common/logging.h"
#include "Common/statefulentity.h"
#include "Common/version.h"
#include "Encryption/secretsmanager.h"
#include "Helpers/constants.h"
#include "Helpers/jsonobjectmap.h"
#include "Helpers/localconfig.h"
#include "Models/Connectivity/proxysettings.h"

#if defined(Q_OS_MAC)
#  define DEFAULT_EXIFTOOL "/usr/bin/exiftool"
#elif defined(APPVEYOR)
#  define DEFAULT_EXIFTOOL "c:/projects/xpiks-deps/windows-3rd-party-bin/exiftool.exe"
#else
#  define DEFAULT_EXIFTOOL "exiftool"
#endif

#define SETTINGS_FILE "settings.json"

#define EXPERIMENTAL_KEY "experimental"

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
#define DEFAULT_USE_SIMPLIFIED_UI false

#define VERBOSE_UPLOAD_STARTDATE "verboseUploadStart"

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

#define SETTINGS_EPSILON 1e-9

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

    SettingsModel::SettingsModel(Common::ISystemEnvironment &environment,
                                 QObject *parent) :
        QObject(parent),
        Common::DelayedActionEntity(SETTINGS_SAVING_INTERVAL, SETTINGS_DELAY_TIMES),
        m_State("settings", environment),
        m_Config(environment.path({SETTINGS_FILE}),
                 environment.getIsInMemoryOnly()),
        m_SettingsMap(std::make_shared<Helpers::JsonObjectMap>()),
        m_ExperimentalMap(std::make_shared<Helpers::JsonObjectMap>()),
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
        m_ExiftoolPathChanged(false),
        m_UseSimplifiedUI(DEFAULT_USE_SIMPLIFIED_UI)
    {
    }

    void SettingsModel::saveLocale() {
        LOG_DEBUG << "#";
        QMutexLocker locker(&m_SettingsMutex);
        Q_UNUSED(locker);
        m_SettingsMap->setValue(Constants::selectedLocale, m_SelectedLocale);
        sync();
    }

    void SettingsModel::initializeConfigs() {
        LOG_DEBUG << "#";

        m_State.init();

        m_SettingsMap = m_Config.readMap();

        if (m_SettingsMap->containsValue(EXPERIMENTAL_KEY)) {
            QJsonValue experimental = m_SettingsMap->value(EXPERIMENTAL_KEY);
            if (experimental.isObject()) {
                QJsonObject experimentalJson = experimental.toObject();
                m_ExperimentalMap = std::make_shared<Helpers::JsonObjectMap>(experimentalJson);
            }
        }
    }

    void SettingsModel::clearLegacyUploadInfos() {
        LOG_DEBUG << "#";

        if (m_SettingsMap->containsValue(Constants::legacyUploadHosts)) {
            LOG_INFO << "Removing legacy upload hosts info";
            QString empty = "";
            m_SettingsMap->setValue(Constants::legacyUploadHosts, empty);
            m_SettingsMap->deleteValue(Constants::legacyUploadHosts);
            sync();
        }
    }

    Common::WordAnalysisFlags SettingsModel::getFlags() const {
        Common::WordAnalysisFlags result = Common::WordAnalysisFlags::None;
        if (getUseSpellCheck()) {
            Common::SetFlag(result, Common::WordAnalysisFlags::Spelling);
        }

        if (getDetectDuplicates()) {
            Common::SetFlag(result, Common::WordAnalysisFlags::Stemming);
        }
        return result;
    }

    void SettingsModel::moveSetting(QSettings &oldSettings, const QString &oldKey, const char *newKey, int type) {
        if (!oldSettings.contains(oldKey)) {
            LOG_WARNING << oldKey << "key is missing";
            return;
        }

        auto oldValue = oldSettings.value(oldKey);
        if (oldValue.convert(type)) {
            auto newValue = QJsonValue::fromVariant(oldValue);
            m_SettingsMap->setValue(newKey, newValue);
        } else {
            LOG_WARNING << "Failed to convert value in key" << oldKey;
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
        m_SettingsMap->setValue(Constants::proxyHost, serializeProxyForSettings(proxySettings));
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
        m_SettingsMap->setValue(Constants::masterPasswordHash, "");
        m_SettingsMap->setValue(Constants::useMasterPassword, false);
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

    void SettingsModel::doReadAllValues() {
        using namespace Constants;

        setExifToolPath(m_SettingsMap->stringValue(pathToExifTool, DEFAULT_EXIFTOOL));
        setUploadTimeout(m_SettingsMap->intValue(oneUploadSecondsTimeout, DEFAULT_UPLOAD_TIMEOUT));
        setMustUseMasterPassword(m_SettingsMap->boolValue(useMasterPassword, DEFAULT_USE_MASTERPASSWORD));
        setMustUseConfirmations(m_SettingsMap->boolValue(useConfirmationDialogs, DEFAULT_USE_CONFIRMATIONS));
        setSaveSession(m_SettingsMap->boolValue(saveSession, DEFAULT_SAVE_SESSION));
        setSaveBackups(m_SettingsMap->boolValue(saveBackups, DEFAULT_SAVE_BACKUPS));
        setKeywordSizeScale(m_SettingsMap->doubleValue(keywordSizeScale, DEFAULT_KEYWORD_SIZE_SCALE));
        setDismissDuration(m_SettingsMap->intValue(dismissDuration, DEFAULT_DISMISS_DURATION));
        setMaxParallelUploads(m_SettingsMap->intValue(maxParallelUploads, DEFAULT_MAX_PARALLEL_UPLOADS));
        setFitSmallPreview(m_SettingsMap->boolValue(fitSmallPreview, DEFAULT_FIT_SMALL_PREVIEW));
        setSearchUsingAnd(m_SettingsMap->boolValue(searchUsingAnd, DEFAULT_SEARCH_USING_AND));
        setSearchByFilepath(m_SettingsMap->boolValue(searchByFilepath, DEFAULT_SEARCH_BY_FILEPATH));
        setScrollSpeedScale(m_SettingsMap->doubleValue(scrollSpeedSensivity, DEFAULT_SCROLL_SPEED_SCALE));
        setUseSpellCheck(m_SettingsMap->boolValue(useSpellCheck, DEFAULT_USE_SPELL_CHECK));
        setDetectDuplicates(m_SettingsMap->boolValue(detectDuplicates, DEFAULT_DETECT_DUPLICATES));
        setUserStatistics(m_SettingsMap->boolValue(userStatistics, DEFAULT_COLLECT_USER_STATISTICS));
        setCheckForUpdates(m_SettingsMap->boolValue(checkForUpdates, DEFAULT_CHECK_FOR_UPDATES));
        setAutoDownloadUpdates(m_SettingsMap->boolValue(autoDownloadUpdates, DEFAULT_AUTO_DOWNLOAD_UPDATES));
        setAutoFindVectors(m_SettingsMap->boolValue(autoFindVectors, DEFAULT_AUTO_FIND_VECTORS));
        setSelectedLocale(m_SettingsMap->stringValue(selectedLocale, DEFAULT_LOCALE));
        setSelectedThemeIndex(m_SettingsMap->intValue(selectedThemeIndex, DEFAULT_SELECTED_THEME_INDEX));
        setUseKeywordsAutoComplete(m_SettingsMap->boolValue(useKeywordsAutoComplete, DEFAULT_USE_KEYWORDS_AUTO_COMPLETE));
        setUsePresetsAutoComplete(m_SettingsMap->boolValue(usePresetsAutoComplete, DEFAULT_USE_PRESETS_AUTO_COMPLETE));
        setUseExifTool(m_SettingsMap->boolValue(useExifTool, DEFAULT_USE_EXIFTOOL));
        setUseProxy(m_SettingsMap->boolValue(useProxy, DEFAULT_USE_PROXY));
        setVerboseUpload(m_SettingsMap->boolValue(verboseUpload, DEFAULT_VERBOSE_UPLOAD));

        setUseProgressiveSuggestionPreviews(m_ExperimentalMap->boolValue(useProgressiveSuggestionPreviews, DEFAULT_USE_PROGRESSIVE_SUGGESTION_PREVIEWS));
        setProgressiveSuggestionIncrement(m_ExperimentalMap->intValue(progressiveSuggestionIncrement, DEFAULT_PROGRESSIVE_SUGGESTION_INCREMENT));
        setUseDirectExiftoolExport(m_ExperimentalMap->boolValue(useDirectExiftoolExport, DEFAULT_USE_DIRECT_EXIFTOOL_EXPORT));
        setUseAutoImport(m_ExperimentalMap->boolValue(useAutoImport, DEFAULT_USE_AUTOIMPORT));
        setUseSimplifiedUI(m_ExperimentalMap->boolValue(useSimplifiedUI, DEFAULT_USE_SIMPLIFIED_UI));

        deserializeProxyFromSettings(m_SettingsMap->stringValue(proxyHost, DEFAULT_PROXY_HOST));

        setAutoCacheImages(m_SettingsMap->boolValue(cacheImagesAutomatically, DEFAULT_AUTO_CACHE_IMAGES));

        consolidateSettings();
        resetChangeStates();
    }

    void SettingsModel::consolidateSettings() {
        do {
            if (getVerboseUpload()) {
                const QDateTime dtNow = QDateTime::currentDateTime();
                const QString verboseUploadStart = m_State.getString(VERBOSE_UPLOAD_STARTDATE);
                const QDateTime verboseUploadStartDateTime = QDateTime::fromString(verboseUploadStart, Qt::ISODate);
                if (!verboseUploadStartDateTime.isValid()) {
                    LOG_WARNING << "Cannot parse verbose upload start datetime:" << verboseUploadStart;
                    break;
                }

                const qint64 daysPassed = verboseUploadStartDateTime.daysTo(dtNow);
                LOG_DEBUG << "Verbose upload is ON for" << daysPassed << "day(s)";

                if (daysPassed > 5) {
                    LOG_INFO << "Resetting stale verbose upload to improve performance";
                    setVerboseUpload(false);
                }
            }
        } while (false);
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
        setUseSimplifiedUI(DEFAULT_USE_SIMPLIFIED_UI);

        justChanged();

        resetChangeStates();
    }

    void SettingsModel::doSaveAllValues() {
        LOG_DEBUG << "#";
        using namespace Constants;

        m_SettingsMap->setValue(pathToExifTool, m_ExifToolPath);
        m_SettingsMap->setValue(oneUploadSecondsTimeout, m_UploadTimeout);
        m_SettingsMap->setValue(useMasterPassword, m_MustUseMasterPassword);
        m_SettingsMap->setValue(useConfirmationDialogs, m_MustUseConfirmations);
        m_SettingsMap->setValue(saveSession, m_SaveSession);
        m_SettingsMap->setValue(saveBackups, m_SaveBackups);
        m_SettingsMap->setValue(keywordSizeScale, m_KeywordSizeScale);
        m_SettingsMap->setValue(dismissDuration, m_DismissDuration);
        m_SettingsMap->setValue(maxParallelUploads, m_MaxParallelUploads);
        m_SettingsMap->setValue(fitSmallPreview, m_FitSmallPreview);
        m_SettingsMap->setValue(searchUsingAnd, m_SearchUsingAnd);
        m_SettingsMap->setValue(searchByFilepath, m_SearchByFilepath);
        m_SettingsMap->setValue(scrollSpeedSensivity, m_ScrollSpeedScale);
        m_SettingsMap->setValue(useSpellCheck, m_UseSpellCheck);
        m_SettingsMap->setValue(detectDuplicates, m_DetectDuplicates);
        m_SettingsMap->setValue(userStatistics, m_UserStatistics);
        m_SettingsMap->setValue(checkForUpdates, m_CheckForUpdates);
        m_SettingsMap->setValue(autoDownloadUpdates, m_AutoDownloadUpdates);
        m_SettingsMap->setValue(autoFindVectors, m_AutoFindVectors);
        m_SettingsMap->setValue(selectedThemeIndex, m_SelectedThemeIndex);
        m_SettingsMap->setValue(useKeywordsAutoComplete, m_UseKeywordsAutoComplete);
        m_SettingsMap->setValue(usePresetsAutoComplete, m_UsePresetsAutoComplete);
        m_SettingsMap->setValue(useExifTool, m_UseExifTool);
        m_SettingsMap->setValue(useProxy, m_UseProxy);
        m_SettingsMap->setValue(proxyHost, serializeProxyForSettings(m_ProxySettings));
        m_SettingsMap->setValue(cacheImagesAutomatically, m_AutoCacheImages);
        m_SettingsMap->setValue(verboseUpload, m_VerboseUpload);

        m_ExperimentalMap->setValue(useProgressiveSuggestionPreviews, m_UseProgressiveSuggestionPreviews);
        m_ExperimentalMap->setValue(progressiveSuggestionIncrement, m_ProgressiveSuggestionIncrement);
        m_ExperimentalMap->setValue(useDirectExiftoolExport, m_UseDirectExiftoolExport);
        m_ExperimentalMap->setValue(useAutoImport, m_UseAutoImport);
        m_ExperimentalMap->setValue(useSimplifiedUI, m_UseSimplifiedUI);

        if (!m_MustUseMasterPassword) {
            m_SettingsMap->setValue(masterPasswordHash, "");
        }

        if (m_VerboseUpload) {
            if (!m_State.contains(VERBOSE_UPLOAD_STARTDATE)) {
                const QDateTime dtNow = QDateTime::currentDateTimeUtc();
                QString dateTimeString = dtNow.toString(Qt::ISODate);
                m_State.setValue(VERBOSE_UPLOAD_STARTDATE, dateTimeString);
                m_State.sync();
            }
        } else {
            if (m_State.contains(VERBOSE_UPLOAD_STARTDATE)) {
                m_State.remove(VERBOSE_UPLOAD_STARTDATE);
                m_State.sync();
            }
        }

        sync();
    }

    void SettingsModel::afterSaveHandler() {
        LOG_DEBUG << "#";

        bool requiresSpellCheck = false;

        if (m_UseSpellCheckChanged) {
            if (!getUseSpellCheck()) {
                emit spellCheckDisabled();
            } else {
                requiresSpellCheck = true;
            }
        }

        if (m_DetectDuplicatesChanged) {
            if (!getDetectDuplicates()) {
                emit duplicatesCheckDisabled();
            } else {
                requiresSpellCheck = true;
            }
        }

        if (requiresSpellCheck) {
        }

        if (m_ExiftoolPathChanged) {
            emit exiftoolSettingChanged(m_ExifToolPath);
        }

        resetChangeStates();

        emit keywordSizeScaleChanged(m_KeywordSizeScale);
        emit settingsUpdated();
    }

    void SettingsModel::resetChangeStates() {
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

        path += QChar('/') + QLatin1String(Constants::WHATS_NEW_FILENAME);
        path = QDir::cleanPath(path);
#else
        // path inside appimage
        path = QCoreApplication::applicationDirPath() + QChar('/') + Constants::WHATS_NEW_FILENAME;
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

    void SettingsModel::userAgreeHandler() {
        LOG_DEBUG << "#";
        m_State.setValue(Constants::userConsent, true);
        m_State.sync();
    }

    void SettingsModel::setUseMasterPassword(bool value) {
        LOG_DEBUG << "#";
        m_SettingsMap->setValue(Constants::useMasterPassword, value);
    }

    void SettingsModel::setMasterPasswordHash(QString const &hash) {
        LOG_DEBUG << "#";
        m_SettingsMap->setValue(Constants::masterPasswordHash, hash);
    }

    void SettingsModel::setUserAgentId(const QString &id) {
        LOG_DEBUG << "#";
        m_State.setValue(Constants::userAgentId, id);
        m_State.sync();
    }

    void SettingsModel::onMasterPasswordSet(Encryption::SecretsManager &secretsManager) {
        LOG_INFO << "Master password changed";

        setMasterPasswordHash(secretsManager.getMasterPasswordHash());
        setUseMasterPassword(true);
        setMustUseMasterPassword(true);
    }

    void SettingsModel::onMasterPasswordUnset(bool firstTime) {
        setMustUseMasterPassword(!firstTime);
        raiseMasterPasswordSignal();
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

    void SettingsModel::setVerboseUpload(bool verboseUpload) {
        if (m_VerboseUpload != verboseUpload) {
            m_VerboseUpload = verboseUpload;
            emit verboseUploadChanged(verboseUpload);
            justChanged();
        }
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

    void SettingsModel::setUseSimplifiedUI(bool value) {
        if (m_UseSimplifiedUI == value)
            return;

        m_UseSimplifiedUI = value;
        emit useSimplifiedUIChanged(value);
        justChanged();
    }

    void SettingsModel::onRecommendedExiftoolFound(const QString &path) {
        LOG_INFO << path;
        if (path.isEmpty()) { return; }

        QString existingExiftoolPath = getExifToolPath();

        if (existingExiftoolPath != path) {
            LOG_DEBUG << "Setting exiftool path to recommended";
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

    void SettingsModel::sync() {
        LOG_DEBUG << "Syncing settings";

        QJsonObject settingsJson = m_SettingsMap->json();
        settingsJson[EXPERIMENTAL_KEY] = m_ExperimentalMap->json();

        QJsonDocument doc;
        doc.setObject(settingsJson);

        m_Config.writeConfig(doc);
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


/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "uploadinforepository.h"

#include <cstddef>

#include <QAbstractItemModel>
#include <QByteArray>
#include <QDataStream>
#include <QFlags>
#include <QIODevice>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QLatin1String>
#include <QList>
#include <QQmlEngine>
#include <QStringList>
#include <QVector>
#include <QtDebug>
#include <QtGlobal>

#include "Common/delayedactionentity.h"
#include "Common/isystemenvironment.h"
#include "Common/logging.h"
#include "Connectivity/ftphelpers.h"
#include "Encryption/secretsmanager.h"
#include "Helpers/asynccoordinator.h"
#include "Helpers/localconfig.h"
#include "Microstocks/stockftpoptions.h"
#include "Microstocks/stocksftplistmodel.h"
#include "Models/Connectivity/uploadinfo.h"
#include "Services/AutoComplete/autocompletemodel.h"
#include "Services/AutoComplete/stringsautocompletemodel.h"

#define UPLOAD_INFO_SAVE_TIMEOUT 3000
#define UPLOAD_INFO_DELAYS_COUNT 10

#define FTP_DESTINATIONS QLatin1String("destinations")
#define FTP_HOST_KEY QLatin1String("host")
#define FTP_USERNAME_KEY QLatin1String("user")
#define FTP_PASS_KEY QLatin1String("pass")
#define FTP_TITLE_KEY QLatin1String("title")
#define FTP_CREATE_ZIP QLatin1String("zip")
#define FTP_DISABLE_PASSIVE QLatin1String("nopassive")
#define FTP_DISABLE_EPSV QLatin1String("noepsv")
#define FTP_ISSELECTED QLatin1String("selected")
#define FTP_VECTORS_FIRST QLatin1String("vectorder")
#define FTP_IMAGES_DIR QLatin1String("imgdir")
#define FTP_VECTORS_DIR QLatin1String("vecdir")
#define FTP_VIDEO_DIR QLatin1String("viddir")

#define UPLOAD_INFOS_FILE "uploadinfos.json"

#define OVERWRITE_KEY "overwrite"
#define OVERWRITE_CSV_PLANS false

namespace Models {
    void serializeUploadInfos(const std::vector<std::shared_ptr<UploadInfo> > &uploadInfos, QJsonObject &result) {
        QJsonArray uploadInfoArray;

        for (auto &uploadInfo: uploadInfos) {
            QJsonObject object;

            object.insert(FTP_TITLE_KEY, uploadInfo->getTitle());
            object.insert(FTP_HOST_KEY, uploadInfo->getHost());
            object.insert(FTP_USERNAME_KEY, uploadInfo->getUsername());
            QString password = uploadInfo->getAnyPassword();
            QString rawPassword = QString::fromLatin1(password.toUtf8().toBase64());
            object.insert(FTP_PASS_KEY, rawPassword);
            object.insert(FTP_CREATE_ZIP, uploadInfo->getZipBeforeUpload());
            object.insert(FTP_DISABLE_PASSIVE, uploadInfo->getDisableFtpPassiveMode());
            object.insert(FTP_DISABLE_EPSV, uploadInfo->getDisableEPSV());
            object.insert(FTP_ISSELECTED, uploadInfo->getIsSelected());
            object.insert(FTP_VECTORS_FIRST, uploadInfo->getVectorFirst());
            object.insert(FTP_IMAGES_DIR, uploadInfo->getImagesDir());
            object.insert(FTP_VECTORS_DIR, uploadInfo->getVectorsDir());
            object.insert(FTP_VIDEO_DIR, uploadInfo->getVideosDir());

            uploadInfoArray.append(object);
        }

        result.insert(FTP_DESTINATIONS, uploadInfoArray);
    }

    bool tryParseHost(const QJsonObject &element, std::shared_ptr<UploadInfo> &uploadInfo) {
        bool parsed = false;

        auto destination = std::make_shared<UploadInfo>();

        do {
            QJsonValue titleValue = element.value(FTP_TITLE_KEY);
            if (!titleValue.isString()) { break; }
            destination->setTitle(titleValue.toString());

            QJsonValue hostValue = element.value(FTP_HOST_KEY);
            if (!hostValue.isString()) { break; }
            destination->setHost(hostValue.toString());

            QJsonValue usernameValue = element.value(FTP_USERNAME_KEY);
            if (usernameValue.isString()) {
                destination->setUsername(usernameValue.toString());
            }

            if (destination->isEmpty()) { break; }

            QJsonValue passwordValue = element.value(FTP_PASS_KEY);
            if (passwordValue.isString()) {
                QString rawPassword = passwordValue.toString();
                QByteArray base64Password = QByteArray::fromBase64(rawPassword.toLatin1());
                QString password = QString::fromUtf8(base64Password);
                destination->setPassword(password);
            }

            QJsonValue createZipValue = element.value(FTP_CREATE_ZIP);
            if (createZipValue.isBool()) {
                destination->setZipBeforeUpload(createZipValue.toBool(false));
            }

            QJsonValue disablePassiveValue = element.value(FTP_DISABLE_PASSIVE);
            if (disablePassiveValue.isBool()) {
                destination->setDisableFtpPassiveMode(disablePassiveValue.toBool(false));
            }

            QJsonValue disableEpsvValue = element.value(FTP_DISABLE_EPSV);
            if (disableEpsvValue.isBool()) {
                destination->setDisableEPSV(disableEpsvValue.toBool(false));
            }

            QJsonValue isSelectedValue = element.value(FTP_ISSELECTED);
            if (isSelectedValue.isBool()) {
                destination->setIsSelected(isSelectedValue.toBool(false));
            }

            QJsonValue vectorFirstValue = element.value(FTP_VECTORS_FIRST);
            if (vectorFirstValue.isBool()) {
                destination->setVectorsFirst(vectorFirstValue.toBool(false));
            }

            QJsonValue imagesDirValue = element.value(FTP_IMAGES_DIR);
            if (imagesDirValue.isString()) {
                destination->setImagesDir(imagesDirValue.toString());
            }

            QJsonValue vectorsDirValue = element.value(FTP_VECTORS_DIR);
            if (vectorsDirValue.isString()) {
                destination->setVectorsDir(vectorsDirValue.toString());
            }

            QJsonValue videosDirValue = element.value(FTP_VIDEO_DIR);
            if (videosDirValue.isString()) {
                destination->setVideosDir(videosDirValue.toString());
            }

            uploadInfo.swap(destination);
            parsed = true;
        } while(false);

        return parsed;
    }

    void parseUploadInfos(const QJsonObject &root, std::vector<std::shared_ptr<UploadInfo> > &uploadInfos) {
        if (!root.contains(FTP_DESTINATIONS)) { return; }
        QJsonValue destinationsObject = root.value(FTP_DESTINATIONS);
        if (!destinationsObject.isArray()) { return; }
        QJsonArray hostsArray = destinationsObject.toArray();
        const int size = hostsArray.size();

        for (int i = 0; i < size; i++) {
            QJsonValue element = hostsArray.at(i);
            if (!element.isObject()) { continue; }

            std::shared_ptr<UploadInfo> item;
            if (tryParseHost(element.toObject(), item)) {
                uploadInfos.emplace_back(item);
            }
        }
    }

    UploadInfoRepository::UploadInfoRepository(Common::ISystemEnvironment &environment,
                                               Encryption::SecretsManager &secretsManager,
                                               QObject *parent):
        QAbstractListModel(parent),
        Common::DelayedActionEntity(UPLOAD_INFO_SAVE_TIMEOUT, UPLOAD_INFO_DELAYS_COUNT),
        m_Environment(environment),
        m_LocalConfig(environment.path({UPLOAD_INFOS_FILE}),
                      environment.getIsInMemoryOnly()),
        m_StocksFtpList(environment),
        m_SecretsManager(secretsManager),
        m_CurrentIndex(0),
        m_EmptyPasswordsMode(false)
    {
        QObject::connect(this, &UploadInfoRepository::backupRequired, &UploadInfoRepository::onBackupRequired);

        QObject::connect(&m_StocksFtpList, &Microstocks::StocksFtpListModel::stocksListUpdated,
                         this, &UploadInfoRepository::onStocksListUpdated);

        QObject::connect(&m_StocksCompletionSource, &AutoComplete::StringsAutoCompleteModel::completionAccepted,
                         this, &UploadInfoRepository::onCompletionSelected);

        QObject::connect(&m_SecretsManager, &Encryption::SecretsManager::beforeMasterPasswordChange,
                         this, &Models::UploadInfoRepository::onBeforeMasterPasswordChanged);
        QObject::connect(&m_SecretsManager, &Encryption::SecretsManager::afterMasterPasswordReset,
                         this, &Models::UploadInfoRepository::onAfterMasterPasswordReset);
    }

    UploadInfoRepository::~UploadInfoRepository() { m_UploadInfos.clear();  }

    void UploadInfoRepository::initFromString(const QString &savedString) {
        LOG_DEBUG << "#";
        if (savedString.isEmpty()) { return; }

        QByteArray originalData;
        originalData.append(savedString.toLatin1());
        QByteArray result = QByteArray::fromBase64(originalData);

        // bad type QList instead of QVector
        // but users already have this
        QList<QHash<int, QString> > items;
        QDataStream stream(&result, QIODevice::ReadOnly);
        stream >> items;

        int length = items.length();
        m_UploadInfos.reserve(length);

        for (int i = 0; i < length; ++i) {
            auto &hash = items.at(i);
            m_UploadInfos.emplace_back(std::make_shared<UploadInfo>(hash));
        }

        LOG_INFO << length << "item(s) found";

        justChanged();
    }

    void UploadInfoRepository::initializeConfig() {
        LOG_DEBUG << "#";
        QJsonDocument config = m_LocalConfig.readConfig();

        decltype(m_UploadInfos) tempInfos;
        if (config.isObject()) {
            parseUploadInfos(config.object(), tempInfos);
            LOG_INFO << "Parsed" << tempInfos.size() << "upload host(s)";
        }

        if (!tempInfos.empty()) {
            m_UploadInfos.swap(tempInfos);
        }
    }

    void UploadInfoRepository::initializeStocksList(Helpers::AsyncCoordinator &initCoordinator,
                                                    Connectivity::IRequestsService &requestsService) {
        LOG_DEBUG << "#";

        Helpers::AsyncCoordinatorLocker locker(initCoordinator);
        Q_UNUSED(locker);
        Helpers::AsyncCoordinatorUnlocker unlocker(initCoordinator);
        Q_UNUSED(unlocker);

        m_StocksFtpList.initializeConfigs(requestsService);
    }

    void UploadInfoRepository::removeItem(int row) {
        beginRemoveRows(QModelIndex(), row, row);
        {
            removeInnerItem(row);
        }
        endRemoveRows();
        emit infosCountChanged();

        justChanged();
    }

    void UploadInfoRepository::addItem() {
        int lastIndex = (int)m_UploadInfos.size();

        LOG_INFO << lastIndex;
        beginInsertRows(QModelIndex(), lastIndex, lastIndex);
        {
            m_UploadInfos.emplace_back(std::make_shared<UploadInfo>());
        }
        endInsertRows();

        emit infosCountChanged();

        justChanged();
    }

    int UploadInfoRepository::getSelectedInfosCount() const {
        int selectedCount = 0;

        for (auto &info: m_UploadInfos) {
            if (info->getIsSelected()) {
                selectedCount++;
            }
        }

        return selectedCount;
    }

    QString UploadInfoRepository::getAgenciesWithMissingDetails() {
        QStringList items;

        for (auto &info: m_UploadInfos) {
            if (info->getIsSelected() && info->isSomethingMissing()) {
                items.append(info->getTitle());
            }
        }

        return items.join(", ");
    }

    void UploadInfoRepository::updateProperties(int itemIndex) {
        QModelIndex itemModelIndex = index(itemIndex);
        // could be any role - just to triger Advanced tab checkbox connections
        emit dataChanged(itemModelIndex, itemModelIndex, QVector<int>() << ZipBeforeUploadRole);
    }

    void UploadInfoRepository::setCurrentIndex(int index) {
        m_CurrentIndex = index;
    }

    // mp == master password
    void UploadInfoRepository::initializeAccounts(bool mpIsCorrectOrEmpty) {
        this->setEmptyPasswordsMode(!mpIsCorrectOrEmpty);
        if (!mpIsCorrectOrEmpty) {
            this->backupAndDropRealPasswords();
        }
    }

    void UploadInfoRepository::finalizeAccounts() {
        if (m_EmptyPasswordsMode) {
            this->restoreRealPasswords();
        }

        m_CurrentIndex = 0;
    }

    QObject *UploadInfoRepository::getStocksCompletionObject() {
        QObject *result = &m_StocksCompletionSource;
        QQmlEngine::setObjectOwnership(result, QQmlEngine::CppOwnership);
        return result;
    }

    void UploadInfoRepository::backupAndDropRealPasswords() {
        for (auto &info: m_UploadInfos) {
            info->backupPassword();
            info->dropPassword();
        }
    }

    void UploadInfoRepository::restoreRealPasswords() {
        for (auto &info: m_UploadInfos) {
            info->restorePassword();
        }
    }

    void UploadInfoRepository::updatePercentages() {
        emit dataChanged(index(0), index((int)m_UploadInfos.size() - 1), QVector<int>() << PercentRole);
    }

    void UploadInfoRepository::resetPercents() {
        LOG_DEBUG << "#";
        for (auto &info: m_UploadInfos) {
            info->resetPercent();
        }
    }

#if defined(INTEGRATION_TESTS) || defined(UI_TESTS)
    void UploadInfoRepository::clearHosts() {
        LOG_DEBUG << "#";
        beginResetModel();
        {
            m_UploadInfos.clear();
        }
        endResetModel();
    }
#endif

    std::vector<std::shared_ptr<UploadInfo>> UploadInfoRepository::retrieveSelectedUploadInfos() const {
        std::vector<std::shared_ptr<UploadInfo>> uploadInfos;
        uploadInfos.reserve(m_UploadInfos.size());

        for (auto &info: m_UploadInfos) {
            if (info->getIsSelected()) {
                uploadInfos.push_back(info);
            }
        }

        return uploadInfos;
    }

    bool UploadInfoRepository::isZippingRequired() const {
        bool anyZipNeeded = false;

        for (auto &info: m_UploadInfos) {
            if (info->getIsSelected() && info->getZipBeforeUpload()) {
                anyZipNeeded = true;
                LOG_DEBUG << "at least for" << info->getHost();
                break;
            }
        }

        return anyZipNeeded;
    }

    std::shared_ptr<UploadInfo> UploadInfoRepository::tryFindItemByHost(const QString &stockAddress) {
        std::shared_ptr<UploadInfo> result;

        for (auto &info: m_UploadInfos) {
            if (Connectivity::sanitizeHost(info->getHost()) == stockAddress) {
                result = info;
                break;
            }
        }

        return result;
    }

    int UploadInfoRepository::rowCount(const QModelIndex &parent) const {
        Q_UNUSED(parent);
        return (int)m_UploadInfos.size();
    }

    QVariant UploadInfoRepository::data(const QModelIndex &index, int role) const {
        int row = index.row();

        if (row < 0 || (size_t)row >= m_UploadInfos.size()) {
            return QVariant();
        }

        auto &uploadInfo = m_UploadInfos.at(row);

        switch (role) {
            case TitleRole:
                return uploadInfo->getTitle();
            case HostRole:
                return uploadInfo->getHost();
            case UsernameRole:
                return uploadInfo->getUsername();
            case PasswordRole: {
                const QString &encodedPassword = uploadInfo->getPassword();
                QString password = m_SecretsManager.decodePassword(encodedPassword);
                return password;
            }
            case IsSelectedRole:
                return uploadInfo->getIsSelected();
            case ZipBeforeUploadRole:
                return uploadInfo->getZipBeforeUpload();
            case PercentRole: {
            double percent = uploadInfo->getPercent();
            return ((1e-7 < percent) && (percent < 1.0)) ? 1.0 : percent;
        }
            /*case FtpPassiveModeRole:
                return uploadInfo->getFtpPassiveMode();*/
            case DisableFtpPassiveModeRole:
                return uploadInfo->getDisableFtpPassiveMode();
            case DisableEPSVRole:
                return uploadInfo->getDisableEPSV();
            case VectorFirstRole:
                return uploadInfo->getVectorFirst();
            default:
                return QVariant();
        }
    }

    Qt::ItemFlags UploadInfoRepository::flags(const QModelIndex &index) const {
        int row = index.row();

        if (row < 0 || (size_t)row >= m_UploadInfos.size()) {
            return Qt::ItemIsEnabled;
        }

        return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
    }

    bool UploadInfoRepository::setData(const QModelIndex &index, const QVariant &value, int role) {
        int row = index.row();

        if (row < 0 || (size_t)row >= m_UploadInfos.size()) {
            return false;
        }

        auto &uploadInfo = m_UploadInfos.at(row);
        QString title;
        int roleToUpdate = 0;
        bool needToUpdate = false;

        switch (role) {
            case EditTitleRole:
                roleToUpdate = TitleRole;
                title = value.toString();
                needToUpdate = uploadInfo->setTitle(title);
                // hack for updating checkbox binding
                if (!needToUpdate) {
                    needToUpdate = true;
                    roleToUpdate = ZipBeforeUploadRole;
                }

                break;
            case EditHostRole:
                roleToUpdate = HostRole;
                needToUpdate = uploadInfo->setHost(value.toString().simplified());
                break;
            case EditUsernameRole:
                roleToUpdate = UsernameRole;
                needToUpdate = uploadInfo->setUsername(value.toString().simplified());
                break;
            case EditPasswordRole: {
                roleToUpdate = PasswordRole;
                QString rawPassword = value.toString();
                QString encodedPassword = m_SecretsManager.encodePassword(rawPassword);
                // skip needUpdate
                uploadInfo->setPassword(encodedPassword);
                break;
            }
            case EditIsSelectedRole:
                roleToUpdate = IsSelectedRole;
                needToUpdate = uploadInfo->setIsSelected(value.toBool());
                break;
            case EditZipBeforeUploadRole:
                roleToUpdate = ZipBeforeUploadRole;
                needToUpdate = uploadInfo->setZipBeforeUpload(value.toBool());
                break;
            /*case EditFtpPassiveModeRole:
                roleToUpdate = FtpPassiveModeRole;
                needToUpdate = uploadInfo->setFtpPassiveMode(value.toBool());*/
            case EditDisableFtpPassiveModeRole:
                roleToUpdate = DisableFtpPassiveModeRole;
                needToUpdate = uploadInfo->setDisableFtpPassiveMode(value.toBool());
                break;
            case EditDisableEPSVRole:
                roleToUpdate = DisableEPSVRole;
                needToUpdate = uploadInfo->setDisableEPSV(value.toBool());
                break;
            case EditVectorFirstRole:
                roleToUpdate = EditVectorFirstRole;
                needToUpdate = uploadInfo->setVectorsFirst(value.toBool());
                break;
            default:
                return false;
        }

        if (needToUpdate) {
            emit dataChanged(index, index, QVector<int>() << roleToUpdate);
        }

        justChanged();
        return true;
    }

    void UploadInfoRepository::onBeforeMasterPasswordChanged(const QString &oldMasterPassword,
                                                             const QString &newMasterPassword) {
        LOG_INFO << m_UploadInfos.size() << "item(s)";

        for (auto &info: m_UploadInfos) {
            if (info->hasPassword()) {
                QString newPassword = m_SecretsManager.recodePassword(
                                          info->getPassword(), oldMasterPassword, newMasterPassword);
                info->setPassword(newPassword);
            }
        }

        justChanged();
    }

    void UploadInfoRepository::onAfterMasterPasswordReset() {
        LOG_INFO << "#";
        for (auto &info: m_UploadInfos) {
            info->dropPassword();
        }
    }

    void UploadInfoRepository::onCompletionSelected(int completionID) {
        QString title = m_StocksCompletionSource.getCompletion(completionID);
        LOG_INFO << "Completion selected for" << title;
        auto ftpOptions = m_StocksFtpList.findFtpOptions(title);
        if (ftpOptions) {
            if ((0 <= m_CurrentIndex) && (m_CurrentIndex < (int)m_UploadInfos.size())) {
                auto &current = m_UploadInfos.at(m_CurrentIndex);
                current->setFtpOptions(*ftpOptions);
                justChanged();
                QModelIndex currIndex = this->index(m_CurrentIndex);
                emit dataChanged(currIndex, currIndex);
            }
        } else {
            LOG_WARNING << "Failed to find" << title;
        }
    }

    void UploadInfoRepository::onBackupRequired() {
        LOG_DEBUG << "#";
        saveUploadInfos();
    }

    void UploadInfoRepository::onStocksListUpdated() {
        LOG_DEBUG << "#";

        QStringList stocks = m_StocksFtpList.getStockNamesList();
        m_StocksCompletionSource.setStrings(stocks);

        for (auto &item: m_UploadInfos) {
            auto ftpOptions = m_StocksFtpList.findFtpOptions(item->getTitle());
            if (ftpOptions != nullptr) {
                if (ftpOptions->m_FtpAddress.contains(item->getHost()) ||
                        item->getHost().contains(ftpOptions->m_FtpAddress)) {
                    LOG_INFO << "Found match. Updating" << item->getTitle() << "data...";
                    item->setImagesDir(ftpOptions->m_ImagesDir);
                    item->setVectorsDir(ftpOptions->m_VectorsDir);
                    item->setVideosDir(ftpOptions->m_VideosDir);
                    item->setZipBeforeUpload(ftpOptions->m_ZipVector);
                } else {
                    LOG_WARNING << "Match found, but address is different:" << item->getHost();
                }
            }
        }

        justChanged();
    }

    bool UploadInfoRepository::saveUploadInfos() {
        LOG_DEBUG << "#";
        QJsonObject uploadInfosObject;
        serializeUploadInfos(m_UploadInfos, uploadInfosObject);

        QJsonDocument doc;
        doc.setObject(uploadInfosObject);
        bool success = m_LocalConfig.writeConfig(doc);
        return success;
    }

    QHash<int, QByteArray> UploadInfoRepository::roleNames() const {
        QHash<int, QByteArray> roles;
        roles[TitleRole] = "title";
        roles[HostRole] = "host";
        roles[UsernameRole] = "username";
        roles[PasswordRole] = "password";
        roles[EditTitleRole] = "edittitle";
        roles[EditHostRole] = "edithost";
        roles[EditUsernameRole] = "editusername";
        roles[EditPasswordRole] = "editpassword";
        roles[IsSelectedRole] = "isselected";
        roles[EditIsSelectedRole] = "editisselected";
        roles[ZipBeforeUploadRole] = "zipbeforeupload";
        roles[EditZipBeforeUploadRole] = "editzipbeforeupload";
        roles[PercentRole] = "percent";
        /*roles[FtpPassiveModeRole] = "ftppassivemode";
           roles[EditFtpPassiveModeRole] = "editftppassivemode";*/
        roles[DisableFtpPassiveModeRole] = "disablepassivemode";
        roles[EditDisableFtpPassiveModeRole] = "editdisablepassivemode";
        roles[DisableEPSVRole] = "disableEPSV";
        roles[EditDisableEPSVRole] = "editdisableEPSV";
        roles[VectorFirstRole] = "vectorfirst";
        roles[EditVectorFirstRole] = "editvectorfirst";
        return roles;
    }

    void UploadInfoRepository::removeInnerItem(int row) {
        m_UploadInfos.erase(m_UploadInfos.begin() + row);
    }
}

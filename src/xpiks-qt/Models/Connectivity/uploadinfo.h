/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef UPLOADINFO
#define UPLOADINFO

#include <QObject>
#include <QHash>
#include <QMutex>
#include <QString>
#include <QByteArray>
#include <cmath>
#include <Microstocks/stockftpoptions.h>

#define BOOL_TO_STR(condition) ((condition) ? QLatin1String("true") : QLatin1String("false"))

namespace Models {
    class UploadInfo : public QObject {
        Q_OBJECT

    private:
        enum UField {
            TitleField = 0,
            HostField = 1,
            UsernameField = 2,
            PasswordField = 3,
            DirectoryField = 4,
            ZipPreviewAndVectorField = 5,
            /*DEPRECATED*/FtpPassiveModeField = 6,
            DisableFtpPassiveModeField = 7,
            IsSelectedField = 8,
            DisableEPSVField = 9,
            ImagesDirField = 10,
            VectorsDirField = 11,
            VideosDirField = 12
        };

    public:
        UploadInfo():
            m_Percent(0),
            m_ZipBeforeUpload(false),
            m_IsSelected(false),
            m_DisableFtpPassiveMode(false),
            m_DisableEPSV(false),
            m_VectorFirst(false)
        {
            m_Title = QObject::tr("Untitled");
        }

        // legacy import
        UploadInfo(const QHash<int, QString> &items) :
            m_Percent(0),
            m_ZipBeforeUpload(false),
            m_IsSelected(false),
            m_DisableFtpPassiveMode(false),
            m_DisableEPSV(false),
            m_VectorFirst(false)
        {
            QString emptyString = QString::fromLatin1("");
            m_Title = items.value(TitleField, QObject::tr("Untitled"));
            m_Host = items.value(HostField, emptyString);
            m_Username = items.value(UsernameField, emptyString);
            m_EncodedPassword = items.value(PasswordField, emptyString);
            m_ZipBeforeUpload = items.value(ZipPreviewAndVectorField, "false") == QLatin1String("true");
            m_DisableFtpPassiveMode = items.value(DisableFtpPassiveModeField, "false") == QLatin1String("true");
            m_IsSelected = items.value(IsSelectedField, "false") == QLatin1String("true");
            m_DisableEPSV = items.value(DisableFtpPassiveModeField, "false") == QLatin1String("true");
        }

    signals:
        void progressChanged(double percent);

    public:
        const QString &getTitle() const { return m_Title; }
        const QString &getHost() const { return m_Host; }
        const QString &getUsername() const { return m_Username; }
        QString getPassword() { QString result; m_Mutex.lock(); result = m_EncodedPassword; m_Mutex.unlock(); return result; }
        bool hasPassword() const { return !m_EncodedPassword.isEmpty(); }
        bool getIsSelected() const { return m_IsSelected; }
        bool getZipBeforeUpload() const { return m_ZipBeforeUpload; }
        bool isSomethingMissing() const { return m_EncodedPassword.isEmpty() || m_Host.isEmpty() || m_Username.isEmpty(); }
        bool isEmpty() const { return m_EncodedPassword.isEmpty() && m_Host.isEmpty() && m_Username.isEmpty() &&
                    (m_Title.isEmpty() || m_Title == QObject::tr("Untitled")); }
        double getPercent() const { return m_Percent; }
        bool getDisableFtpPassiveMode() const { return m_DisableFtpPassiveMode; }
        bool getDisableEPSV() const { return m_DisableEPSV; }
        QString getAnyPassword() { return m_EncodedPassword.isEmpty() ? m_EncodedPasswordBackup : m_EncodedPassword; }
        bool getVectorFirst() const { return m_VectorFirst; }
        const QString &getImagesDir() const { return m_ImagesDir; }
        const QString &getVectorsDir() const { return m_VectorsDir; }
        const QString &getVideosDir() const { return m_VideosDir; }

    public:
        bool setTitle(const QString &value) { bool result = m_Title != value; m_Title = value; return result; }
        bool setHost(const QString &value) { bool result = m_Host != value; m_Host = value; return result; }
        bool setUsername(const QString &value) { bool result = m_Username != value; m_Username = value; return result; }
        bool setPassword(const QString &value) {
            bool result = value != m_EncodedPassword;
            if (result) {
                m_Mutex.lock();
                {
                    result = value != m_EncodedPassword;
                    if (result) {
                        m_EncodedPassword = value;
                    }
                }
                m_Mutex.unlock();
            }
            return result;
        }
        bool setIsSelected(bool value) { bool result = m_IsSelected != value; m_IsSelected = value; return result; }
        bool setZipBeforeUpload(bool value) { bool result = m_ZipBeforeUpload != value; m_ZipBeforeUpload = value; return result; }
        void restorePassword() { m_EncodedPassword = m_EncodedPasswordBackup; }
        void backupPassword() { m_EncodedPasswordBackup = m_EncodedPassword; }
        void dropPassword() { m_EncodedPassword = ""; }
        void setPercent(double value) {
            if (fabs(m_Percent - value) > 0.0001) {
                m_Percent = value;
                emit progressChanged(value);
            }
        }
        void resetPercent() { m_Percent = 0.0; }
        bool setDisableFtpPassiveMode(bool value) {
            bool result = m_DisableFtpPassiveMode != value;
            m_DisableFtpPassiveMode = value;
            return result;
        }
        bool setDisableEPSV(bool value) {
            bool result = m_DisableEPSV != value;
            m_DisableEPSV = value;
            return result;
        }
        bool setVectorsFirst(bool value) {
            bool result = m_VectorFirst != value;
            m_VectorFirst = value;
            return result;
        }
        bool setImagesDir(const QString &value) {
            bool result = m_ImagesDir != value;
            m_ImagesDir = value;
            return result;
        }
        bool setVectorsDir(const QString &value) {
            bool result = m_VectorsDir != value;
            m_VectorsDir = value;
            return result;
        }
        bool setVideosDir(const QString &value) {
            bool result = m_VideosDir != value;
            m_VideosDir = value;
            return result;
        }
        void setFtpOptions(const Microstocks::StockFtpOptions &ftpOptions) {
            m_Title = ftpOptions.m_Title;
            m_Host = ftpOptions.m_FtpAddress;
            m_ImagesDir = ftpOptions.m_ImagesDir;
            m_VectorsDir = ftpOptions.m_VectorsDir;
            m_VideosDir = ftpOptions.m_VideosDir;
            m_ZipBeforeUpload = ftpOptions.m_ZipVector;
        }

    private:
        QMutex m_Mutex;
        QString m_Title;
        QString m_Host;
        QString m_Username;
        QString m_EncodedPassword;
        QString m_ImagesDir;
        QString m_VectorsDir;
        QString m_VideosDir;
        // used for backup when MP is incorrect
        QString m_EncodedPasswordBackup;
        volatile double m_Percent;
        bool m_ZipBeforeUpload;
        bool m_IsSelected;
        bool m_DisableFtpPassiveMode;
        bool m_DisableEPSV;
        bool m_VectorFirst;
    };
}

#endif // UPLOADINFO


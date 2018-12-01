/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2017 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef CURLFTPUPLOADER_H
#define CURLFTPUPLOADER_H

#include <atomic>
#include <memory>

#include <QObject>
#include <QString>

namespace libxpks {
    namespace net {
        class UploadBatch;

        class CurlProgressReporter : public QObject {
            Q_OBJECT
        public:
            CurlProgressReporter(void *curl);

        public:
            void updateProgress(double ultotal, double ulnow);
            void *getCurl() const { return m_Curl; }
            double getLastTime() const { return m_LastTime; }
            void setLastTime(double value) { m_LastTime = value; }
            bool cancelRequested() const { return m_Cancel; }

        signals:
            void progressChanged(double percentsDone);

        public slots:
            void cancelHandler();

        private:
            double m_LastTime;
            void *m_Curl;
            std::atomic_bool m_Cancel;
        };

        class CurlFtpUploader : public QObject
        {
            Q_OBJECT
        public:
            explicit CurlFtpUploader(const std::shared_ptr<UploadBatch> &batchToUpload, QObject *parent = 0);

        public:
            void uploadBatch();

        signals:
            void uploadStarted();
            void progressChanged(double prevPercents, double newPercents);
            void uploadFinished(bool anyErrors);
            void transferFailed(const QString &filepath, const QString &host);
            void cancelCurrentUpload();

        public slots:
            void cancel();

        private slots:
            void reportCurrentFileProgress(double percent);

        private:
            std::shared_ptr<UploadBatch> m_BatchToUpload;
            std::atomic_int m_UploadedCount;
            std::atomic_bool m_Cancel;
            double m_LastPercentage;
            int m_TotalCount;
        };
    }
}

#endif // CURLFTPUPLOADER_H

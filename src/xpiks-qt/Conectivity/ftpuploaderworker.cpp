/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2016 Taras Kushnir <kushnirTV@gmail.com>
 *
 * Xpiks is distributed under the GNU General Public License, version 3.0
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "ftpuploaderworker.h"
#include <QSemaphore>
#include <QDebug>
#include "curlftpuploader.h"
#include "../Models/uploadinfo.h"

namespace Conectivity {
    FtpUploaderWorker::FtpUploaderWorker(QSemaphore *uploadSemaphore,
                                         Encryption::SecretsManager *secretsManager,
                                         UploadBatch *batch,
                                         Models::UploadInfo *uploadInfo,
                                         QObject *parent) :
        QObject(parent),
        m_UploadSemaphore(uploadSemaphore),
        m_SecretsManager(secretsManager),
        m_UploadBatch(batch),
        m_UploadInfo(uploadInfo)
    {
    }

    void FtpUploaderWorker::process() {
        const QString &host = m_UploadBatch->getContext()->m_Host;

        qDebug() << "Waiting for the semaphore" << host;
        m_UploadSemaphore->acquire();

        if (!m_Cancel) {
            qInfo() << "Starting upload to" << host;
            doUpload();
        } else {
            qInfo() << "Upload cancelled before start for" << host;
        }

        m_UploadSemaphore->release();
        qDebug() << "Released semaphore" << host;

        emit stopped();
    }

    void FtpUploaderWorker::progressChangedHandler(double oldPercents, double newPercents) {
        Q_UNUSED(oldPercents);
        m_UploadInfo->setPercent(newPercents);
    }

    void FtpUploaderWorker::doUpload() {
        CurlFtpUploader ftpUploader(m_UploadBatch);

        QObject::connect(&ftpUploader, SIGNAL(uploadStarted()), this, SIGNAL(uploadStarted()));
        QObject::connect(&ftpUploader, SIGNAL(uploadFinished(bool)), this, SIGNAL(uploadFinished(bool)));
        QObject::connect(&ftpUploader, SIGNAL(progressChanged(double, double)), this, SIGNAL(progressChanged(double, double)));
        QObject::connect(&ftpUploader, SIGNAL(progressChanged(double, double)), this, SLOT(progressChangedHandler(double,double)));
        QObject::connect(this, SIGNAL(workerCancelled()), &ftpUploader, SLOT(cancel()));

        ftpUploader.uploadBatch();
    }
}


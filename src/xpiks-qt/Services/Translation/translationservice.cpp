/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "translationservice.h"

#include <memory>

#include <QChar>
#include <QStringList>
#include <QThread>
#include <QtDebug>
#include <QtGlobal>

#include "Common/logging.h"
#include "Helpers/asynccoordinator.h"
#include "Services/Translation/translationquery.h"
#include "Services/Translation/translationworker.h"

namespace Translation {
    TranslationService::TranslationService(QObject *parent) :
        QObject(parent),
        m_TranslationWorker(nullptr)
    {
    }

    void TranslationService::startService(Helpers::AsyncCoordinator &coordinator) {
        if (m_TranslationWorker != nullptr) {
            LOG_WARNING << "Attempt to start running worker";
            return;
        }

        Helpers::AsyncCoordinatorLocker locker(coordinator);
        Q_UNUSED(locker);

        m_TranslationWorker = new TranslationWorker(coordinator);

        QThread *thread = new QThread();
        m_TranslationWorker->moveToThread(thread);

        QObject::connect(thread, &QThread::started, m_TranslationWorker, &TranslationWorker::process);
        QObject::connect(m_TranslationWorker, &TranslationWorker::stopped, thread, &QThread::quit);

        QObject::connect(m_TranslationWorker, &TranslationWorker::stopped,
                         m_TranslationWorker, &TranslationWorker::deleteLater);
        QObject::connect(thread, &QThread::finished, thread, &QThread::deleteLater);

        QObject::connect(m_TranslationWorker, &TranslationWorker::stopped,
                         this, &TranslationService::workerFinished);

        QObject::connect(m_TranslationWorker, &TranslationWorker::destroyed,
                         this, &TranslationService::workerDestroyed);

        LOG_DEBUG << "starting thread...";
        thread->start();
    }

    void TranslationService::stopService() {
        LOG_DEBUG << "#";
        if (m_TranslationWorker != nullptr) {
            m_TranslationWorker->stopWorking();
        } else {
            LOG_WARNING << "TranslationWorker was nullptr";
        }
    }

    bool TranslationService::isBusy() const {
        bool isBusy = (m_TranslationWorker != nullptr) && m_TranslationWorker->hasPendingJobs();
        return isBusy;
    }

    void TranslationService::selectDictionary(const QString &dictionaryPath) {
        Q_ASSERT(m_TranslationWorker != nullptr);
        m_TranslationWorker->selectDictionary(dictionaryPath);
    }

    void TranslationService::translate(const QString &what) {
        LOG_INFO << what;

        QStringList words = what.split(QChar::Space, QString::SkipEmptyParts);
        QString wordToTranslate = words.isEmpty() ? "" : words.last();

        std::shared_ptr<TranslationQuery> query(new TranslationQuery(wordToTranslate.simplified().toLower()),
                                                [](TranslationQuery *tq) { tq->deleteLater(); });
        QObject::connect(query.get(), &TranslationQuery::translationAvailable,
                         this, &TranslationService::translationAvailable);

        m_TranslationWorker->submitItem(query);
    }

    bool TranslationService::retrieveTranslation(const QString &what, QString &translation) {
        Q_ASSERT(m_TranslationWorker != nullptr);
        if (m_TranslationWorker == nullptr) { return false; }

        QStringList words = what.split(QChar::Space, QString::SkipEmptyParts);
        QString wordToTranslate = words.isEmpty() ? "" : words.last();

        bool found = m_TranslationWorker->retrieveTranslation(wordToTranslate.simplified().toLower(),
                                                              translation);
        return found;
    }

    void TranslationService::workerFinished() {
        LOG_DEBUG << "#";
    }

    void TranslationService::workerDestroyed(QObject *object) {
        Q_UNUSED(object);
        LOG_DEBUG << "#";
        m_TranslationWorker = nullptr;
    }
}

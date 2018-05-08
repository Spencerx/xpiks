/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "translationservice.h"
#include <QThread>
#include "translationworker.h"
#include "translationquery.h"
#include "../Helpers/asynccoordinator.h"

namespace Translation {
    TranslationService::TranslationService(TranslationManager &manager, QObject *parent) :
        QObject(parent),
        m_TranslationManager(manager),
        m_TranslationWorker(nullptr),
        m_RestartRequired(false)
    {
    }

    void TranslationService::startService(const std::shared_ptr<Common::ServiceStartParams> &params) {
        if (m_TranslationWorker != nullptr) {
            LOG_WARNING << "Attempt to start running worker";
            return;
        }

        auto coordinatorParams = std::dynamic_pointer_cast<Helpers::AsyncCoordinatorStartParams>(params);
        Helpers::AsyncCoordinator *coordinator = nullptr;
        if (coordinatorParams) { coordinator = coordinatorParams->m_Coordinator; }

        Helpers::AsyncCoordinatorLocker locker(coordinator);
        Q_UNUSED(locker);

        m_TranslationWorker = new TranslationWorker(coordinator);

        QThread *thread = new QThread();
        m_TranslationWorker->moveToThread(thread);

        QObject::connect(thread, &QThread::started, m_TranslationWorker, &TranslationWorker::process);
        QObject::connect(m_TranslationWorker, &TranslationWorker::stopped, thread, &QThread::quit);

        QObject::connect(m_TranslationWorker, &TranslationWorker::stopped, m_TranslationWorker, &TranslationWorker::deleteLater);
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
            LOG_WARNING << "TranslationWorker was NULL";
        }
    }

    bool TranslationService::isBusy() const {
        bool isBusy = (m_TranslationWorker != NULL) && m_TranslationWorker->hasPendingJobs();
        return isBusy;
    }

    void TranslationService::submitItem(QString *item) {
        Q_UNUSED(item);
    }

    void TranslationService::submitItem(QString *item, Common::flag_t flags) {
        Q_UNUSED(flags);
        this->submitItem(item);
    }

    void TranslationService::submitItems(const std::vector<QString *> &items) {
        Q_UNUSED(items);
#if QT_NO_DEBUG
        LOG_WARNING << "Cannot use this API for translation";
#else
        Q_ASSERT(false);
#endif
    }

    void TranslationService::selectDictionary(const QString &dictionaryPath) {
        Q_ASSERT(m_TranslationWorker != nullptr);
        m_TranslationWorker->selectDictionary(dictionaryPath);
    }

    void TranslationService::translate(const QString &what) {
        LOG_INFO << what;

        QStringList words = what.split(QChar::Space, QString::SkipEmptyParts);
        QString wordToTranslate = words.isEmpty() ? "" : words.last();

        std::shared_ptr<TranslationQuery> query(new TranslationQuery(wordToTranslate),
                                                [](TranslationQuery *tq) { tq->deleteLater(); });
        QObject::connect(query.get(), &TranslationQuery::translationAvailable, &m_TranslationManager, &TranslationManager::translationArrived);

        m_TranslationWorker->submitItem(query);
    }

    void TranslationService::workerFinished() {
        LOG_INFO << "#";
    }

    void TranslationService::workerDestroyed(QObject *object) {
        Q_UNUSED(object);
        LOG_DEBUG << "#";
        m_TranslationWorker = nullptr;

        if (m_RestartRequired) {
            LOG_INFO << "Restarting worker...";
            startService(std::shared_ptr<Common::ServiceStartParams>());
            m_RestartRequired = false;
        }
    }
}

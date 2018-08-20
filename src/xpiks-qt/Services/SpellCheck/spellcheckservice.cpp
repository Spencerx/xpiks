/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "spellcheckservice.h"
#include <QThread>
#include "spellcheckitem.h"
#include "userdictionary.h"
#include <Artworks/artworkmetadata.h>
#include <Common/logging.h>
#include <Common/flags.h>
#include <Artworks/artworkssnapshot.h>
#include <Helpers/cpphelpers.h>

namespace SpellCheck {
    SpellCheckService::SpellCheckService(Common::ISystemEnvironment &environment,
                                         Common::IFlagsProvider<Common::WordAnalysisFlags> &analysisFlagsProvider):
        m_Environment(environment),
        m_SpellCheckWorker(NULL),
        m_AnalysisFlagsProvider(analysisFlagsProvider),
        m_IsStopped(false)
    {
    }

    SpellCheckService::~SpellCheckService() {
        LOG_DEBUG << "#";
    }

    void SpellCheckService::startService(Helpers::AsyncCoordinator &initCoordinator,
                                         UserDictionary &userDictionary,
                                         Warnings::WarningsService &warningsService) {
        if (m_SpellCheckWorker != NULL) {
            LOG_WARNING << "Attempt to start running worker";
            return;
        }

        Helpers::AsyncCoordinatorLocker locker(initCoordinator);
        Q_UNUSED(locker);

        m_SpellCheckWorker = new SpellCheckWorker(getDictsRoot(),
                                                  m_Environment,
                                                  userDictionary,
                                                  warningsService,
                                                  initCoordinator);

        QThread *thread = new QThread();
        m_SpellCheckWorker->moveToThread(thread);

        QObject::connect(thread, &QThread::started, m_SpellCheckWorker, &SpellCheckWorker::process);
        QObject::connect(m_SpellCheckWorker, &SpellCheckWorker::stopped, thread, &QThread::quit);

        QObject::connect(m_SpellCheckWorker, &SpellCheckWorker::stopped, m_SpellCheckWorker, &SpellCheckWorker::deleteLater);
        QObject::connect(thread, &QThread::finished, thread, &QThread::deleteLater);

        QObject::connect(this, &SpellCheckService::cancelSpellChecking,
                         m_SpellCheckWorker, &SpellCheckWorker::cancel);

        QObject::connect(m_SpellCheckWorker, &SpellCheckWorker::queueIsEmpty,
                         this, &SpellCheckService::spellCheckQueueIsEmpty);

        QObject::connect(m_SpellCheckWorker, &SpellCheckWorker::stopped,
                         this, &SpellCheckService::workerFinished);
        QObject::connect(m_SpellCheckWorker, &SpellCheckWorker::destroyed,
                         this, &SpellCheckService::workerDestroyed);

        LOG_DEBUG << "starting thread...";
        thread->start();

        m_IsStopped = false;

        emit serviceAvailable();
    }

    void SpellCheckService::stopService() {
        LOG_DEBUG << "#";
        if (m_SpellCheckWorker != NULL) {
            m_SpellCheckWorker->stopWorking();
        } else {
            LOG_WARNING << "SpellCheckWorker is NULL";
        }

        m_IsStopped = true;
    }

#ifdef INTEGRATION_TESTS
    bool SpellCheckService::isBusy() const {
        bool isBusy = (m_SpellCheckWorker != NULL) && (m_SpellCheckWorker->hasPendingJobs());
        return isBusy;
    }
#endif

    quint32 SpellCheckService::submitItems(const std::vector<std::shared_ptr<Artworks::IBasicModelSource> > &items, Common::SpellCheckFlags flags) {
        LOG_INFO << items.size() << "items to check";
        if (m_SpellCheckWorker == NULL) { return INVALID_BATCH_ID; }
        if (m_IsStopped) { return INVALID_BATCH_ID; }

        std::vector<std::shared_ptr<SpellCheckItem>> spellCheckItems;
        spellCheckItems.reserve(items.size());

        auto deleter = [](SpellCheckItem *spi) {
            LOG_INTEGRATION_TESTS << "Delete later for multiple spellcheck item";
            spi->disconnect();
            spi->deleteLater();
        };

        for (auto &item: items) {
            spellCheckItems.push_back(
                        std::shared_ptr<SpellCheckItem>(
                            new SpellCheckItem(item, flags, m_AnalysisFlagsProvider.getFlags()),
                            deleter));
        }

        auto batchID = m_SpellCheckWorker->submitItems(spellCheckItems);
        m_SpellCheckWorker->submitSeparator();
        return batchID;
    }

    void SpellCheckService::submitItem(const std::shared_ptr<Artworks::IBasicModelSource> &item, Common::SpellCheckFlags flags) {
        if (m_SpellCheckWorker == NULL) { return; }
        if (m_IsStopped) { return; }

        Q_ASSERT(item != nullptr);

        std::shared_ptr<SpellCheckItem> spellCheckItem(
                    new SpellCheckItem(item, flags, m_AnalysisFlagsProvider.getFlags()),
                    [](SpellCheckItem *spi) {
            LOG_INTEGRATION_TESTS << "Delete later for single spellcheck item";
            spi->disconnect();
            spi->deleteLater();
        });
        m_SpellCheckWorker->submitItem(spellCheckItem);
    }

    QStringList SpellCheckService::suggestCorrections(const QString &word) const {
        if (m_SpellCheckWorker == NULL) {
            LOG_DEBUG << "Worker is null";
            return QStringList();
        }

        QStringList corrections = m_SpellCheckWorker->retrieveCorrections(word);
        return corrections;
    }

#ifdef INTEGRATION_TESTS
    int SpellCheckService::getSuggestionsCount() {
        return m_SpellCheckWorker->getSuggestionsCount();
    }
#endif

    void SpellCheckService::cancelCurrentBatch() {
        LOG_DEBUG << "#";
        if (m_SpellCheckWorker == NULL) { return; }

        m_SpellCheckWorker->cancelPendingJobs();
    }

    bool SpellCheckService::hasAnyPending() {
        bool hasPending = false;

        if (m_SpellCheckWorker != NULL) {
            hasPending = m_SpellCheckWorker->hasPendingJobs();
        }

        return hasPending;
    }

    void SpellCheckService::workerFinished() {
        LOG_DEBUG << "#";
    }

    void SpellCheckService::workerDestroyed(QObject *object) {
        Q_UNUSED(object);
        LOG_DEBUG << "#";
        m_SpellCheckWorker = NULL;
    }

    QString SpellCheckService::getDictsRoot() const {
        QString resourcesPath;
#ifdef INTEGRATION_TESTS
        resourcesPath = STRINGIZE(DEPS_DIR);
#else
        resourcesPath = QCoreApplication::applicationDirPath();
#if defined(Q_OS_MAC)
        resourcesPath += "/../Resources/";
#endif
#endif

        resourcesPath += "/dict/";

        return QDir::cleanPath(resourcesPath);
    }
}

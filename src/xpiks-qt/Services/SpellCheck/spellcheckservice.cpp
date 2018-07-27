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
#include <Artworks/artworkmetadata.h>
#include <Common/logging.h>
#include <Common/flags.h>
#include <Models/settingsmodel.h>
#include <Artworks/artworkssnapshot.h>
#include <Helpers/cpphelpers.h>

namespace SpellCheck {
    SpellCheckService::SpellCheckService(Common::ISystemEnvironment &environment,
                                             Warnings::WarningsService &warningsService,
                                             Models::SettingsModel &settingsModel):
        m_Environment(environment),
        m_WarningsService(warningsService),
        m_UserDictionary(environment),
        m_SettingsModel(settingsModel),
        m_RestartRequired(false),
        m_IsStopped(false)
    {
        QObject::connect(&m_UserDictionary, &UserDictionary::sizeChanged,
                         this, &SpellCheckService::userDictWordsNumberChanged);
    }

    void SpellCheckService::handleMessage(const Common::NamedType<Artworks::ArtworkMetadata *, Common::MessageType::SpellCheck> &event) {
        this->submitArtwork(event.get());
    }

    void SpellCheckService::handleMessage(const Common::NamedType<Artworks::BasicKeywordsModel *, Common::MessageType::SpellCheck> &event) {
        this->submitItem(event.get(), Common::SpellCheckFlags::All);
    }

    void SpellCheckService::startService(const std::shared_ptr<Services::ServiceStartParams> &params) {
        if (m_SpellCheckWorker != NULL) {
            LOG_WARNING << "Attempt to start running worker";
            return;
        }

        auto coordinatorParams = std::dynamic_pointer_cast<Helpers::AsyncCoordinatorStartParams>(params);
        Helpers::AsyncCoordinator *coordinator = nullptr;
        if (coordinatorParams) { coordinator = coordinatorParams->m_Coordinator; }

        m_SpellCheckWorker = new SpellCheckWorker(getDictsRoot(),
                                                  m_Environment,
                                                  m_UserDictionary,
                                                  m_WarningsService,
                                                  coordinator);
        Helpers::AsyncCoordinatorLocker locker(coordinator);
        Q_UNUSED(locker);

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

        emit serviceAvailable(m_RestartRequired);
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

    bool SpellCheckService::isBusy() const {
        bool isBusy = (m_SpellCheckWorker != NULL) && (m_SpellCheckWorker->hasPendingJobs());
        return isBusy;
    }

    void SpellCheckService::submitItem(Artworks::BasicKeywordsModel *itemToCheck, Common::SpellCheckFlags flags) {
        if (m_SpellCheckWorker == NULL) { return; }
        if (m_IsStopped) { return; }

        Q_ASSERT(itemToCheck != nullptr);
        LOG_INFO << "flags:" << (int)flags;

        std::shared_ptr<SpellCheckItem> item(new SpellCheckItem(itemToCheck, flags, getWordAnalysisFlags()),
            [](SpellCheckItem *spi) {
            LOG_INTEGRATION_TESTS << "Delete later for single spellcheck item";
            spi->disconnect();
            spi->deleteLater();
        });
        m_SpellCheckWorker->submitItem(item);
    }

    void SpellCheckService::submitArtworks(const Artworks::ArtworksSnapshot &snapshot) {
        this->submitItems(
                    Helpers::map<std::shared_ptr<Artworks::ArtworkMetadataLocker>, Artworks::ArtworkMetadata*>(
                        snapshot.getRawData(),
                        [](const std::shared_ptr<Artworks::ArtworkMetadataLocker> &locker) {
            return locker->getArtworkMetadata();
        }));
    }

    void SpellCheckService::submitArtwork(Artworks::ArtworkMetadata *artwork) {
        if (m_SpellCheckWorker == NULL) { return; }
        if (m_IsStopped) { return; }
        Q_ASSERT(artwork != nullptr);
        if (artwork == nullptr) { return; }

        auto deleter = [](SpellCheckItem *spi) {
            LOG_INTEGRATION_TESTS << "Delete later for multiple spellcheck item";
            spi->disconnect();
            spi->deleteLater();
        };

        std::shared_ptr<SpellCheckItem> item(
                    new ArtworkSpellCheckItem(artwork, Common::SpellCheckFlags::All, getWordAnalysisFlags()),
                    deleter);
        m_SpellCheckWorker->submitItem(item);
    }

    SpellCheckWorker::batch_id_t SpellCheckService::submitItems(const std::vector<Artworks::ArtworkMetadata *> &itemsToCheck) {
        if (m_SpellCheckWorker == NULL) { return INVALID_BATCH_ID; }
        if (m_IsStopped) { return INVALID_BATCH_ID; }

        std::vector<std::shared_ptr<SpellCheckItem> > items;
        const size_t size = itemsToCheck.size();

        items.reserve(size);
        auto deleter = [](SpellCheckItem *spi) {
            LOG_INTEGRATION_TESTS << "Delete later for multiple spellcheck item";
            spi->disconnect();
            spi->deleteLater();
        };

        const Common::WordAnalysisFlags flags = getWordAnalysisFlags();

        for (auto itemToCheck: itemsToCheck) {
            Q_ASSERT(itemToCheck != nullptr);
            std::shared_ptr<SpellCheckItem> item(
                        new ArtworkSpellCheckItem(itemToCheck, Common::SpellCheckFlags::All, flags),
                        deleter);
            items.emplace_back(item);
        }

        LOG_INFO << size << "item(s)";

        auto batchID = m_SpellCheckWorker->submitItems(items);
        m_SpellCheckWorker->submitSeparator();
        return batchID;
    }

    SpellCheckWorker::batch_id_t SpellCheckService::submitItems(const std::vector<Artworks::BasicKeywordsModel *> &itemsToCheck) {
        if (m_SpellCheckWorker == NULL) { return INVALID_BATCH_ID; }
        if (m_IsStopped) { return INVALID_BATCH_ID; }

        std::vector<std::shared_ptr<SpellCheckItem> > items;
        const size_t size = itemsToCheck.size();

        items.reserve(size);
        auto deleter = [](SpellCheckItem *spi) {
            LOG_INTEGRATION_TESTS << "Delete later for multiple spellcheck item";
            spi->disconnect();
            spi->deleteLater();
        };

        const Common::WordAnalysisFlags flags = getWordAnalysisFlags();

        for (auto itemToCheck: itemsToCheck) {
            Q_ASSERT(itemToCheck != nullptr);
            std::shared_ptr<SpellCheckItem> item(
                        new SpellCheckItem(itemToCheck, Common::SpellCheckFlags::All, flags),
                        deleter);
            items.emplace_back(item);
        }

        LOG_INFO << size << "item(s)";

        auto batchID = m_SpellCheckWorker->submitItems(items);
        m_SpellCheckWorker->submitSeparator();
        return batchID;
    }

    // used for spellchecking after adding a word to user dictionary
    void SpellCheckService::submitItems(const std::vector<Artworks::ArtworkMetadata *> &itemsToCheck,
                                          const QStringList &wordsToCheck) {
        if (m_SpellCheckWorker == NULL) { return; }
        if (m_IsStopped) { return; }

        std::vector<std::shared_ptr<SpellCheckItem> > items;
        const size_t size = itemsToCheck.size();

        items.reserve(size);
        auto deleter = [](SpellCheckItem *spi) {
            LOG_INTEGRATION_TESTS << "Delete later for UserDict spelling item";
            spi->disconnect();
            spi->deleteLater();
        };

        const Common::WordAnalysisFlags flags = getWordAnalysisFlags();

        for (auto itemToCheck: itemsToCheck) {
            std::shared_ptr<SpellCheckItem> item(
                        new ArtworkSpellCheckItem(itemToCheck, wordsToCheck, flags),
                        deleter);
            items.emplace_back(item);
        }

        LOG_INFO << size << "item(s)";

        m_SpellCheckWorker->submitItems(items);
        m_SpellCheckWorker->submitSeparator();
    }

    QStringList SpellCheckService::suggestCorrections(const QString &word) const {
        if (m_SpellCheckWorker == NULL) {
            LOG_DEBUG << "Worker is null";
            return QStringList();
        }

        QStringList corrections = m_SpellCheckWorker->retrieveCorrections(word);
        return corrections;
    }

    int SpellCheckService::getUserDictWordsNumber() {
        return m_UserDictionary.size();
    }

    QStringList SpellCheckService::getUserDictionary() {
        return m_UserDictionary.getWords();
    }

#ifdef INTEGRATION_TESTS
    int SpellCheckerService::getSuggestionsCount() {
        return m_SpellCheckWorker->getSuggestionsCount();
    }
#endif

    void SpellCheckService::updateUserDictionary(const QStringList &words) {
        LOG_INFO << words;
        m_UserDictionary.reset(words);
        m_UserDictionary.save();
    }

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

    void SpellCheckService::addWordToUserDictionary(const QString &word) {
        m_UserDictionary.addWord(word);
        m_UserDictionary.save();
    }

    void SpellCheckService::clearUserDictionary() {
        m_UserDictionary.clear();
        m_UserDictionary.save();
    }

    void SpellCheckService::workerFinished() {
        LOG_DEBUG << "#";
    }

    void SpellCheckService::workerDestroyed(QObject *object) {
        Q_UNUSED(object);
        LOG_DEBUG << "#";
        m_SpellCheckWorker = NULL;

        if (m_RestartRequired) {
            LOG_INFO << "Restarting worker...";
            startService(std::shared_ptr<Services::ServiceStartParams>());
            m_RestartRequired = false;
        }
    }

    Common::WordAnalysisFlags SpellCheckService::getWordAnalysisFlags() const {
        Common::WordAnalysisFlags result = Common::WordAnalysisFlags::None;
        if (m_SettingsModel.getUseSpellCheck()) {
            Common::SetFlag(result, Common::WordAnalysisFlags::Spelling);
        }

        if (m_SettingsModel.getDetectDuplicates()) {
            Common::SetFlag(result, Common::WordAnalysisFlags::Stemming);
        }
        return result;
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

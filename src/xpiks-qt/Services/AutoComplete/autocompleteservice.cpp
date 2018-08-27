/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "autocompleteservice.h"
#include <QThread>
#include "autocompleteworker.h"
#include "completionquery.h"
#include <Common/flags.h>
#include <Artworks/basickeywordsmodel.h>
#include <Helpers/asynccoordinator.h>
#include <Models/settingsmodel.h>
#include "keywordsautocompletemodel.h"

namespace AutoComplete {
    AutoCompleteService::AutoCompleteService(Models::SettingsModel &settingsModel,
                                             KeywordsAutoCompleteModel &autoCompleteModel,
                                             QObject *parent):
        QObject(parent),
        m_WorkerThread(nullptr),
        m_AutoCompleteWorker(nullptr),
        m_AutoCompleteModel(autoCompleteModel),
        m_SettingsModel(settingsModel)
    {
    }

    void AutoCompleteService::startService(Helpers::AsyncCoordinator &coordinator,
                                           KeywordsPresets::PresetKeywordsModel &presetsManager) {
        if (m_AutoCompleteWorker != NULL) {
            LOG_WARNING << "Attempt to start running worker";
            return;
        }

        m_AutoCompleteWorker = new AutoCompleteWorker(coordinator, m_AutoCompleteModel, presetsManager);

        QThread *thread = new QThread();
        m_AutoCompleteWorker->moveToThread(thread);

        QObject::connect(thread, &QThread::started, m_AutoCompleteWorker, &AutoCompleteWorker::process);
        QObject::connect(m_AutoCompleteWorker, &AutoCompleteWorker::stopped, thread, &QThread::quit);

        QObject::connect(m_AutoCompleteWorker, &AutoCompleteWorker::stopped, m_AutoCompleteWorker, &AutoCompleteWorker::deleteLater);
        QObject::connect(thread, &QThread::finished, thread, &QThread::deleteLater);

        QObject::connect(this, &AutoCompleteService::cancelAutoCompletion,
                         m_AutoCompleteWorker, &AutoCompleteWorker::cancel);

        QObject::connect(m_AutoCompleteWorker, &AutoCompleteWorker::stopped,
                         this, &AutoCompleteService::workerFinished);

        QObject::connect(m_AutoCompleteWorker, &AutoCompleteWorker::destroyed,
                         this, &AutoCompleteService::workerDestroyed);

        LOG_DEBUG << "starting thread...";
        thread->start();
        m_WorkerThread = thread;

        emit serviceAvailable();
    }

    void AutoCompleteService::stopService() {
        LOG_DEBUG << "#";
        if (m_AutoCompleteWorker != NULL) {
            m_AutoCompleteWorker->stopWorking();
        } else {
            LOG_WARNING << "AutoComplete Worker was NULL";
        }
    }

    bool AutoCompleteService::isBusy() const {
        bool isBusy = (m_AutoCompleteWorker != NULL) && m_AutoCompleteWorker->hasPendingJobs();
        return isBusy;
    }

    void AutoCompleteService::generateCompletions(const QString &prefix, Artworks::BasicKeywordsModel &basicModel) {
        if (m_AutoCompleteWorker == NULL) {
            LOG_WARNING << "Worker is NULL";
            return;
        }

        auto query = createQuery(prefix);
        if (query == nullptr) { return; }
        QObject::connect(query.get(), &CompletionQuery::completionsAvailable,
                         &basicModel, &Artworks::BasicKeywordsModel::completionsAvailable);
        query.get()->moveToThread(m_WorkerThread);

        m_AutoCompleteWorker->submitItem(query);
    }

    void AutoCompleteService::generateCompletions(const QString &prefix) {
        if (m_AutoCompleteWorker == NULL) {
            LOG_WARNING << "Worker is NULL";
            return;
        }

        auto query = createQuery(prefix);
        if (query == nullptr) { return; }
        QObject::connect(query.get(), &CompletionQuery::completionsAvailable,
                         &m_AutoCompleteModel, &KeywordsAutoCompleteModel::completionsAvailable);
        query.get()->moveToThread(m_WorkerThread);

        m_AutoCompleteWorker->submitItem(query);
    }

    std::shared_ptr<CompletionQuery> AutoCompleteService::createQuery(const QString &prefix) {
        const bool completeKeywords = m_SettingsModel.getUseKeywordsAutoComplete();
        const bool completePresets = m_SettingsModel.getUsePresetsAutoComplete();

        LOG_VERBOSE_OR_DEBUG << "Complete keywords:" << completeKeywords << "presets:" << completePresets;

        if (!completeKeywords && !completePresets) {
            LOG_VERBOSE_OR_DEBUG << "Completions are disabled";
            return std::shared_ptr<CompletionQuery>();
        }

        LOG_INFO << "Received:" << prefix;
        QString requestPrefix = prefix.toLower();
        LOG_INFO << "Requesting for" << requestPrefix;

        std::shared_ptr<CompletionQuery> query(new CompletionQuery(requestPrefix),
                                               [](CompletionQuery *q) { q->deleteLater(); });
        query->setCompleteKeywords(completeKeywords);
        query->setCompletePresets(completePresets);
        return query;
    }

    void AutoCompleteService::workerFinished() {
        LOG_INFO << "#";
    }

    void AutoCompleteService::workerDestroyed(QObject *object) {
        Q_UNUSED(object);
        LOG_DEBUG << "#";
        m_AutoCompleteWorker = NULL;
    }
}

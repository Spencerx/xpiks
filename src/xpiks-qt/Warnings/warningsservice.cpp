/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2017 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "warningsservice.h"
#include <QVector>
#include "../Common/defines.h"
#include "warningscheckingworker.h"
#include "../Commands/commandmanager.h"
#include "warningsitem.h"
#include "../Common/flags.h"

namespace Warnings {
    WarningsService::WarningsService(QObject *parent):
        QObject(parent),
        m_WarningsWorker(NULL) {}

    void WarningsService::initWarningsSettings() {
        QTimer::singleShot(1000, this, SLOT(updateWarningsSettings()));
    }

    void WarningsService::startService() {
        m_WarningsWorker = new WarningsCheckingWorker(&m_WarningsSettingsModel);

        QThread *thread = new QThread();
        m_WarningsWorker->moveToThread(thread);

        QObject::connect(thread, SIGNAL(started()), m_WarningsWorker, SLOT(process()));
        QObject::connect(m_WarningsWorker, SIGNAL(stopped()), thread, SLOT(quit()));

        QObject::connect(m_WarningsWorker, SIGNAL(stopped()), m_WarningsWorker, SLOT(deleteLater()));
        QObject::connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));

        QObject::connect(m_WarningsWorker, SIGNAL(destroyed(QObject *)),
                         this, SLOT(workerDestoyed(QObject *)));

        QObject::connect(m_WarningsWorker, SIGNAL(stopped()),
                         this, SLOT(workerStopped()));

#ifdef INTEGRATION_TESTS
        QObject::connect(m_WarningsWorker, SIGNAL(queueIsEmpty()),
                         this, SIGNAL(queueIsEmpty()));
#endif

        LOG_INFO << "Starting worker";

        thread->start();
    }

    void WarningsService::stopService() {
        if (m_WarningsWorker != NULL) {
            LOG_INFO << "Stopping worker";
            m_WarningsWorker->stopWorking();
        } else {
            LOG_WARNING << "Warnings Worker was NULL";
        }
    }

    bool WarningsService::isBusy() const {
        bool isBusy = (m_WarningsWorker != NULL) && (m_WarningsWorker->hasPendingJobs());

        return isBusy;
    }

    void WarningsService::submitItem(Models::ArtworkMetadata *item) {
        if (m_WarningsWorker == NULL) {
            return;
        }

        LOG_INFO << "Submitting one item";

        std::shared_ptr<WarningsItem> wItem(new WarningsItem(item));
        m_WarningsWorker->submitItem(wItem);
    }

    void WarningsService::submitItem(Models::ArtworkMetadata *item, Common::WarningsCheckFlags flags) {
        if (m_WarningsWorker == NULL) {
            return;
        }

        LOG_INFO << "Submitting one item with flags" << Common::warningsFlagToString(flags);

        std::shared_ptr<WarningsItem> wItem(new WarningsItem(item, flags));
        m_WarningsWorker->submitItem(wItem);
    }

    void WarningsService::submitItems(const QVector<Models::ArtworkMetadata *> &items) {
        if (m_WarningsWorker == NULL) {
            return;
        }

        int length = items.length();

        std::vector<std::shared_ptr<WarningsItem> > itemsToSubmit;
        itemsToSubmit.reserve(length);

        for (int i = 0; i < length; ++i) {
            Models::ArtworkMetadata *item = items.at(i);
            itemsToSubmit.emplace_back(new WarningsItem(item));
        }

        LOG_INFO << "Submitting" << length << "item(s)";
        m_WarningsWorker->submitItems(itemsToSubmit);
    }

    void WarningsService::setCommandManager(Commands::CommandManager *commandManager) {
        Common::BaseEntity::setCommandManager(commandManager);

        m_WarningsSettingsModel.setCommandManager(commandManager);
    }

    void WarningsService::workerDestoyed(QObject *object) {
        Q_UNUSED(object);
        LOG_DEBUG << "#";
        m_WarningsWorker = NULL;
    }

    void WarningsService::workerStopped() {
        LOG_DEBUG << "#";
    }

    void WarningsService::updateWarningsSettings() {
        LOG_DEBUG << "#";
        m_WarningsSettingsModel.initializeConfigs();
    }
}

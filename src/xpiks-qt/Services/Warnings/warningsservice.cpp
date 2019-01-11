/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "warningsservice.h"

#include <cstddef>
#include <vector>

#include <QThread>
#include <QtDebug>
#include <QtGlobal>

#include "Artworks/artworkssnapshot.h"
#include "Common/flags.h"
#include "Common/logging.h"
#include "Services/Warnings/warningscheckingworker.h"
#include "Services/Warnings/warningsitem.h"

namespace Warnings {
    class IWarningsItem;

    QString warningsFlagToString(Common::WarningsCheckFlags flags) {
#ifdef QT_DEBUG
        QStringList items;
        if (Common::HasFlag(flags, Common::WarningsCheckFlags::All)) {
            items << "All";
        } else {
            if (Common::HasFlag(flags, Common::WarningsCheckFlags::Metadata)) {
                items << "Metadata";
            } else {
                if (Common::HasFlag(flags, Common::WarningsCheckFlags::Title)) {
                    items << "Title";
                }

                if (Common::HasFlag(flags, Common::WarningsCheckFlags::Description)) {
                    items << "Description";
                }

                if (Common::HasFlag(flags, Common::WarningsCheckFlags::Keywords)) {
                    items << "Keywords";
                }
            }

            if (Common::HasFlag(flags, Common::WarningsCheckFlags::FileProperties)) {
                items << "File";
            }

            if (Common::HasFlag(flags, Common::WarningsCheckFlags::Spelling)) {
                items << "Spelling";
            }
        }

        return items.join(QChar('|'));
#else
        return QString("Release");
#endif
    }

    WarningsService::WarningsService(WarningsSettingsModel &warningsSettingsModel, QObject *parent):
        QObject(parent),
        m_WarningsWorker(nullptr),
        m_WarningsSettingsModel(warningsSettingsModel),
        m_IsStopped(false)
    { }

    void WarningsService::cancelCurrentBatch() {
        LOG_DEBUG << "#";

        if (m_WarningsWorker != nullptr) {
            m_WarningsWorker->cancelPendingJobs();
        }
    }

    void WarningsService::startService() {
        m_WarningsWorker = new WarningsCheckingWorker(m_WarningsSettingsModel);

        QThread *thread = new QThread();
        m_WarningsWorker->moveToThread(thread);

        QObject::connect(thread, &QThread::started, m_WarningsWorker, &WarningsCheckingWorker::process);
        QObject::connect(m_WarningsWorker, &WarningsCheckingWorker::stopped, thread, &QThread::quit);

        QObject::connect(m_WarningsWorker, &WarningsCheckingWorker::stopped, m_WarningsWorker, &WarningsCheckingWorker::deleteLater);
        QObject::connect(thread, &QThread::finished, thread, &QThread::deleteLater);

        QObject::connect(m_WarningsWorker, &WarningsCheckingWorker::destroyed,
                         this, &WarningsService::workerDestoyed);

        QObject::connect(m_WarningsWorker, &WarningsCheckingWorker::stopped,
                         this, &WarningsService::workerStopped);

        QObject::connect(m_WarningsWorker, &WarningsCheckingWorker::queueIsEmpty,
                         this, &WarningsService::queueIsEmpty);

        LOG_INFO << "Starting worker";
        thread->start(QThread::LowestPriority);
    }

    void WarningsService::stopService() {
        LOG_INFO << "Stopping worker";
        if (isRunning()) {
            m_WarningsWorker->stopWorking();
        } else {
            LOG_WARNING << "Warnings Worker was nullptr";
        }

        m_IsStopped = true;
    }

    bool WarningsService::isBusy() const {
        bool isBusy = (m_WarningsWorker != nullptr) && (m_WarningsWorker->hasPendingJobs());
        return isBusy;
    }

    void WarningsService::submitItem(std::shared_ptr<Artworks::ArtworkMetadata> const &item) {
        if (!isRunning()) { return; }
        LOG_INFO << "Submitting one item";
        auto wItem = std::make_shared<WarningsItem>(item);
        m_WarningsWorker->submitItem(wItem);
    }

    void WarningsService::submitItem(std::shared_ptr<Artworks::ArtworkMetadata> const &item,
                                     Common::WarningsCheckFlags flags) {
        if (!isRunning()) { return; }
        LOG_INFO << "flags:" << (int)flags << warningsFlagToString(flags);
        auto wItem = std::make_shared<WarningsItem>(item, flags);
        m_WarningsWorker->submitItem(wItem);
    }

    void WarningsService::submitItems(const Artworks::ArtworksSnapshot &items) {
        if (!isRunning()) { return; }

        const size_t size = items.size();
        std::vector<std::shared_ptr<IWarningsItem> > itemsToSubmit;
        itemsToSubmit.reserve(size);

        for (auto &item: items) {
            itemsToSubmit.emplace_back(std::make_shared<WarningsItem>(item));
        }

        LOG_INFO << "Submitting" << size << "item(s)";
        m_WarningsWorker->submitItems(itemsToSubmit);
        m_WarningsWorker->submitSeparator();
    }

    void WarningsService::workerDestoyed(QObject *object) {
        Q_UNUSED(object);
        LOG_DEBUG << "#";
        m_WarningsWorker = nullptr;
    }

    void WarningsService::workerStopped() {
        LOG_DEBUG << "#";
    }

    bool WarningsService::isRunning() {
        return m_WarningsWorker != nullptr && !m_IsStopped;
    }
}

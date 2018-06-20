/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef ITEMPROCESSINGWORKER_H
#define ITEMPROCESSINGWORKER_H

#include <QWaitCondition>
#include <QMutex>
#include <deque>
#include <memory>
#include <vector>
#include <algorithm>
#include "../Common/flags.h"
#include "../Common/defines.h"
#include "../Common/logging.h"
#include "../Helpers/threadhelpers.h"

namespace Common {
    template<typename ItemType, typename ResultType>
    class ItemProcessingWorker
    {
    public:
        typedef quint32 batch_id_t;
        class WorkItem {
        public:
            WorkItem(std::shared_ptr<ItemType> &item, Common::flag_t flags, batch_id_t id):
                m_Item(std::move(item)),
                m_Flags(flags),
                m_ID(id)
            {}
            WorkItem(Common::flag_t flags):
                m_Flags(flags)
            {}
            WorkItem(){}

        private:
            inline bool getIsSeparatorFlag() const { return Common::HasFlag(m_Flags, FlagIsSeparator); }
            inline bool getIsStopperFlag() const { return Common::HasFlag(m_Flags, FlagIsStopper); }
            inline bool getIsMilestoneFlag() const { return Common::HasFlag(m_Flags, FlagIsMilestone); }

        public:
            bool isStopper() const { return (m_Item.get() == nullptr) && getIsStopperFlag(); }
            bool isSeparator() const { return getIsSeparatorFlag(); }
            bool isMilestone() const { return getIsMilestoneFlag(); }

        public:
            std::shared_ptr<ItemType> m_Item;
            Common::flag_t m_Flags = 0;
            batch_id_t m_ID = INVALID_BATCH_ID;
        };

        class WorkResult {
        public:
            WorkResult(std::shared_ptr<ResultType> &result, batch_id_t batchID):
                m_Result(std::move(result)),
                m_BatchID(batchID)
            { }
        public:
            std::shared_ptr<ResultType> m_Result;
            batch_id_t m_BatchID;
        };

    public:
        ItemProcessingWorker(int delayPeriod = 0xffffffff):
            m_BatchID(1),
            m_MilestoneSize(delayPeriod),
            m_Cancel(false)
        { }

        virtual ~ItemProcessingWorker() { }

    private:
        enum WorkerFlags {
            FlagIsSeparator = 1 << 0,
            FlagIsStopper = 1 << 1,
            FlagIsMilestone = 1 << 2
        };

    protected:

    public:
        void submitSeparator() {
            if (m_Cancel) { return; }

            m_QueueMutex.lock();
            {
                Common::flag_t flags = 0;
                Common::SetFlag(flags, FlagIsSeparator);

                bool wasEmpty = m_Queue.empty();
                m_Queue.emplace_back(WorkItem(flags));

                if (wasEmpty) {
                    m_WaitAnyItem.wakeOne();
                }
            }
            m_QueueMutex.unlock();
        }

        batch_id_t submitItem(const std::shared_ptr<ItemType> &item) {
            if (m_Cancel) {
                return INVALID_BATCH_ID;
            }

            batch_id_t batchID;
            Common::flag_t flags = 0;
            m_QueueMutex.lock();
            {
                batchID = getNextBatchID();
                bool wasEmpty = m_Queue.empty();
                m_Queue.emplace_back(item, flags, batchID);

                if (wasEmpty) {
                    m_WaitAnyItem.wakeOne();
                }
            }
            m_QueueMutex.unlock();

            return batchID;
        }

        batch_id_t submitFirst(const std::shared_ptr<ItemType> &item) {
            if (m_Cancel) {
                return INVALID_BATCH_ID;
            }

            batch_id_t batchID;
            Common::flag_t flags = 0;
            m_QueueMutex.lock();
            {
                batchID = getNextBatchID();
                bool wasEmpty = m_Queue.empty();
                m_Queue.emplace_front(item, flags, batchID);

                if (wasEmpty) {
                    m_WaitAnyItem.wakeOne();
                }
            }
            m_QueueMutex.unlock();

            return batchID;
        }

        batch_id_t submitItems(const std::vector<std::shared_ptr<ItemType> > &items) {
            if (m_Cancel) {
                return INVALID_BATCH_ID;
            }

            batch_id_t batchID;
            Common::flag_t commonFlags = 0;
            m_QueueMutex.lock();
            {
                batchID = getNextBatchID();
                bool wasEmpty = m_Queue.empty();

                const size_t size = items.size();
                for (size_t i = 0; i < size; ++i) {
                    auto &item = items.at(i);

                    Common::flag_t flags = commonFlags;
                    if (i % m_MilestoneSize == 0) { Common::SetFlag(flags, FlagIsMilestone); }

                    m_Queue.emplace_back(item, flags, batchID);
                }

                if (wasEmpty) {
                    m_WaitAnyItem.wakeOne();
                }
            }
            m_QueueMutex.unlock();

            return batchID;
        }

        void cancelPendingJobs() {
            m_QueueMutex.lock();
            {
                m_Queue.clear();
            }
            m_QueueMutex.unlock();

            onQueueIsEmpty();
        }

        void cancelBatch(batch_id_t batchID) {
            if (batchID == INVALID_BATCH_ID) { return; }

            bool isEmpty = false;
            m_QueueMutex.lock();
            {
                m_Queue.erase(std::remove_if(m_Queue.begin(), m_Queue.end(),
                                             [&batchID](const WorkItem &item) {
                    return item.m_ID == batchID;
                }),
                              m_Queue.end());

                isEmpty = m_Queue.empty();
            }
            m_QueueMutex.unlock();

            if (isEmpty) {
                onQueueIsEmpty();
            }
        }

        bool hasPendingJobs() {
            QMutexLocker locker(&m_QueueMutex);
            bool isEmpty = m_Queue.empty();
            return !isEmpty;
        }

        bool isCancelled() const { return m_Cancel; }

        void doWork() {
            if (initWorker()) {
                runWorkerLoop();
            } else {
                m_Cancel = true;
            }

            onWorkerStopped();
        }

        void stopWorking(bool immediately=true) {
            m_Cancel = true;

            m_QueueMutex.lock();
            {
                if (immediately) {
                    m_Queue.clear();
                }

                Common::flag_t flags = 0;
                Common::SetFlag(flags, FlagIsStopper);

                m_Queue.emplace_back(WorkItem(flags));
                m_WaitAnyItem.wakeOne();
            }
            m_QueueMutex.unlock();
        }

        void waitIdle() {
            m_IdleEvent.waitOne();
        }

    protected:
        virtual bool initWorker() { return true; }
        virtual std::shared_ptr<ResultType> processOneItem(WorkItem &workItem) = 0;
        virtual void onQueueIsEmpty() { /* DO NOTHING */ }
        virtual void onWorkerStopped() { /* DO NOTHING */ }
        virtual void onResultsAvailable(const std::deque<WorkResult> &results) { Q_UNUSED(results); }

        void runWorkerLoop() {
            m_IdleEvent.set();

            for (;;) {
                if (m_Cancel) {
                    LOG_INFO << "Cancelled. Exiting...";
                    break;
                }

                bool noMoreItems = false;
                WorkItem workItem;

                m_QueueMutex.lock();
                {
                    while (m_Queue.empty()) {
                        bool waitResult = m_WaitAnyItem.wait(&m_QueueMutex);
                        if (!waitResult) {
                            LOG_WARNING << "Waiting failed for new items";
                        }
                    }

                    workItem = m_Queue.front();
                    m_Queue.pop_front();
                    noMoreItems = m_Queue.empty();
                }
                m_QueueMutex.unlock();

                if (workItem.isStopper()) { break; }

                m_IdleEvent.reset();
                {
                    try {
                        auto result = processOneItem(workItem);
                        m_Results.emplace_back(result, workItem.m_ID);
                    }
                    catch (...) {
                        LOG_WARNING << "Exception while processing item!";
                    }
                }
                m_IdleEvent.set();

                if (noMoreItems) { onQueueIsEmpty(); }
                if (noMoreItems || workItem.isSeparator()) { reportResults(); }
            }
        }

        void reportResults() {
            decltype(m_Results) results;
            results.swap(m_Results);
            onResultsAvailable(results);
        }

    private:
        inline batch_id_t getNextBatchID() {
            batch_id_t id = m_BatchID++;
            return id;
        }

    private:
        Helpers::ManualResetEvent m_IdleEvent;
        QWaitCondition m_WaitAnyItem;
        QMutex m_QueueMutex;
        std::deque<WorkItem> m_Queue;
        std::deque<WorkResult> m_Results;
        batch_id_t m_BatchID;
        unsigned int m_MilestoneSize;
        volatile bool m_Cancel;
    };
}

#endif // ITEMPROCESSINGWORKER_H

/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "undoredomanager.h"
#include "../Common/defines.h"
#include "../Common/logging.h"

void UndoRedo::UndoRedoManager::recordHistoryItem(const std::shared_ptr<Commands::ICommand> &historyItem) {
    LOG_INFO << "History item about to be recorded";

    QMutexLocker locker(&m_Mutex);

    if (!m_HistoryStack.empty()) {
        m_HistoryStack.pop();
        Q_ASSERT(m_HistoryStack.empty());
    }

    m_HistoryStack.push(historyItem);
    emit canUndoChanged();
    emit itemRecorded();
    emit undoDescriptionChanged();
}

bool UndoRedo::UndoRedoManager::undoLastAction() {
    LOG_DEBUG << "#";
    m_Mutex.lock();

    bool anyItem = false;
    anyItem = !m_HistoryStack.empty();

    if (anyItem) {
        std::shared_ptr<Commands::ICommand> historyItem(std::move(m_HistoryStack.top()));
        m_HistoryStack.pop();
        m_Mutex.unlock();

        emit canUndoChanged();
        emit undoDescriptionChanged();
        historyItem->undo();
    } else {
        m_Mutex.unlock();
        LOG_WARNING << "No item for undo";
    }

    return anyItem;
}

void UndoRedo::UndoRedoManager::discardLastAction() {
    LOG_DEBUG << "#";
    m_Mutex.lock();

    if (!m_HistoryStack.empty()) {
        m_HistoryStack.pop();
        bool isNowEmpty = m_HistoryStack.empty();

        m_Mutex.unlock();

        emit canUndoChanged();
        emit undoDescriptionChanged();

        if (isNowEmpty) {
            emit undoStackEmpty();
        }
    } else {
        m_Mutex.unlock();
    }
}

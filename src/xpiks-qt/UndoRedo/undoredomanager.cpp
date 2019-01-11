/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "undoredomanager.h"

#include <utility>

#include <QMutexLocker>
#include <QtDebug>
#include <QtGlobal>

#include "Common/logging.h"
#include "Common/messages.h"
#include "Common/types.h"

void UndoRedo::UndoRedoManager::handleMessage(const Common::NamedType<int, Common::MessageType::UnavailableFiles> &) {
    LOG_DEBUG << "#";
    undoLastAction();
}

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

    bool dummy = false;
    auto item = popLastItem(dummy);
    if (item) {
        item->undo();
    }

    return (item != nullptr);
}

void UndoRedo::UndoRedoManager::discardLastAction() {
    LOG_DEBUG << "#";
    bool emptyNow = false;
    popLastItem(emptyNow);

    if (emptyNow) {
        emit undoStackEmpty();
    }
}

std::shared_ptr<Commands::ICommand> UndoRedo::UndoRedoManager::popLastItem(bool &emptyNow) {
    LOG_DEBUG << "#";
    std::shared_ptr<Commands::ICommand> result;

    m_Mutex.lock();

    bool anyItem = !m_HistoryStack.empty();

    if (anyItem) {
        result = std::move(m_HistoryStack.top());
        m_HistoryStack.pop();
        emptyNow = m_HistoryStack.empty();

        m_Mutex.unlock();

        emit canUndoChanged();
        emit undoDescriptionChanged();
    } else {
        m_Mutex.unlock();
        LOG_WARNING << "No item for undo";
    }

    return result;
}

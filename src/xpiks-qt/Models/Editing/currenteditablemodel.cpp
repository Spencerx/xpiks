/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "currenteditablemodel.h"
#include <Common/logging.h>
#include <Models/Artworks/artworkslistmodel.h>
#include <Models/Editing/combinedartworksmodel.h>
#include <Models/Editing/artworkproxymodel.h>

namespace Models {
    CurrentEditableModel::CurrentEditableModel(QObject *parent) :
        QObject(parent)
    {
    }

    void CurrentEditableModel::handleMessage(const std::shared_ptr<ICurrentEditable> &event) {
        LOG_DEBUG << "#";
        setCurrentEditable(event);
    }

    void CurrentEditableModel::clearCurrentItem() {
        LOG_DEBUG << "#";
        m_CurrentEditable.reset();
        emit currentEditableChanged();
    }

    void CurrentEditableModel::setCurrentEditable(std::shared_ptr<ICurrentEditable> currentEditable) {
        LOG_DEBUG << "#";
        m_CurrentEditable = std::move(currentEditable);
        emit currentEditableChanged();
    }

    void CurrentEditableModel::onClearCurrentEditable() {
        clearCurrentItem();
    }
}

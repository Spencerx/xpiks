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
#include <Commands/appmessages.h>

namespace Models {
    CurrentEditableModel::CurrentEditableModel(Commands::AppMessages &messages,
                                               QObject *parent) :
        QObject(parent)
    {
        messages
                .ofType<std::shared_ptr<ICurrentEditable>>()
                .withID(Commands::AppMessages::RegisterCurrentEditable)
                .addListener(std::bind(&CurrentEditableModel::setCurrentEditable, this));
    }

    void CurrentEditableModel::clearCurrentItem() {
        LOG_DEBUG << "#";
        m_CurrentEditable.reset();
        emit currentEditableChanged();
    }

    void CurrentEditableModel::setCurrentEditable(std::shared_ptr<ICurrentEditable> currentEditable) {
        m_CurrentEditable = std::move(currentEditable);
        emit currentEditableChanged();
    }

    void CurrentEditableModel::onClearCurrentEditable() {
        clearCurrentItem();
    }
}

/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef CURRENTEDITABLEMODEL_H
#define CURRENTEDITABLEMODEL_H

#include <memory>

#include <QObject>
#include <QString>

#include "Common/messages.h"
#include "Models/Editing/icurrenteditable.h"

namespace Models {
    class CurrentEditableModel:
            public QObject,
            public ICurrentEditableSource,
            public Common::MessagesTarget<std::shared_ptr<Models::ICurrentEditable>>
    {
        Q_OBJECT
        Q_PROPERTY(bool isAvailable READ getIsAvailable NOTIFY currentEditableChanged)

    public:
        explicit CurrentEditableModel(QObject *parent = nullptr);

    public:
        bool getIsAvailable() const { return m_CurrentEditable.operator bool(); }
        virtual std::shared_ptr<Models::ICurrentEditable> getCurrentEditable() const override { return m_CurrentEditable; }

    public:
        virtual void handleMessage(const std::shared_ptr<Models::ICurrentEditable> &event) override;

    public:
        void clearCurrentItem();

    private:
        void setCurrentEditable(std::shared_ptr<Models::ICurrentEditable> currentEditable);

    signals:
        void currentEditableChanged();

    public slots:
        void onClearCurrentEditable();

    private:
        std::shared_ptr<Models::ICurrentEditable> m_CurrentEditable;
    };
}

#endif // CURRENTEDITABLEMODEL_H

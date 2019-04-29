/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef LOGGINGWORKER_H
#define LOGGINGWORKER_H

#include <atomic>

#include <QObject>
#include <QString>

namespace Helpers {
    class LoggingWorker : public QObject
    {
        Q_OBJECT
    public:
        explicit LoggingWorker(QObject *parent = nullptr);
        virtual ~LoggingWorker() {}

    signals:
        void stopped();

    public slots:
        void process();
        void cancel();

    private:
        std::atomic_bool m_Cancel;
    };
}

#endif // LOGGINGWORKER_H

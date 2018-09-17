/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef LOGGING_H
#define LOGGING_H

#include <QDebug>
#include "types.h"

#if (QT_VERSION <= QT_VERSION_CHECK(5, 4, 2))
#define qInfo qDebug
#endif

#define LOG_DEBUG qDebug()
#define LOG_INFO qInfo()

#ifdef QT_DEBUG
#define LOG_FOR_DEBUG qDebug()
#else
#define LOG_FOR_DEBUG if (1) {} else qDebug()
#endif

#ifdef CORE_TESTS
#define LOG_CORE_TESTS qDebug()
#else
#define LOG_CORE_TESTS if (1) {} else qDebug()
#endif

#if defined(VERBOSE_LOGGING)
#define LOG_VERBOSE qDebug()
#else
#define LOG_VERBOSE if (1) {} else qDebug()
#endif

#if defined(QT_DEBUG) || defined(VERBOSE_LOGGING)
#define LOG_VERBOSE_OR_DEBUG qDebug()
#else
#define LOG_VERBOSE_OR_DEBUG if (1) {} else qDebug()
#endif

#if defined(FAKE_WARNINGS)
#define LOG_WARNING qInfo() << "FAKE_WARNING"
#else
#define LOG_WARNING qWarning()
#endif

#if defined(INTEGRATION_TESTS) || defined(CORE_TESTS) || defined(UI_TESTS)
#define LOG_FOR_TESTS qDebug()
#else
#define LOG_FOR_TESTS if (1) {} else qDebug()
#endif

QDebug& operator<< (QDebug &debug, const Common::ID_t &t);

#endif // LOGGING_H

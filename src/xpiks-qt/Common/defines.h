/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef DEFINES
#define DEFINES

#include <QtGlobal>
#include <QThread>
#include <QDebug>
#include <QDateTime>

#define STRINGIZE_(x) #x
#define STRINGIZE(x) STRINGIZE_(x)

// 4*(avg word size 10 * avg keywords size 50)
#define MAX_PASTE_SIZE 2000

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

#if defined(INTEGRATION_TESTS) || defined(VERBOSE_LOGGING)
#define LOG_INTEGRATION_TESTS qDebug()
#else
#define LOG_INTEGRATION_TESTS if (1) {} else qDebug()
#endif

#if defined(INTEGRATION_TESTS) || defined(QT_DEBUG) || defined(VERBOSE_LOGGING)
#define LOG_INTEGR_TESTS_OR_DEBUG qDebug()
#else
#define LOG_INTEGR_TESTS_OR_DEBUG if (1) {} else qDebug()
#endif

#if defined(INTEGRATION_TESTS) || defined(CORE_TESTS) || defined(FAKE_WARNINGS)
#define LOG_WARNING qInfo() << "FAKE_WARNING"
#else
#define LOG_WARNING qWarning()
#endif

#if defined(INTEGRATION_TESTS) || defined(CORE_TESTS)
#define LOG_FOR_TESTS qDebug()
#else
#define LOG_FOR_TESTS if (1) {} else qDebug()
#endif

#define PREVIEWOFFSET 25
#define PREVIEWKEYWORDSCOUNT 5

#define DUPLICATEOFFSET 25
#define DUPLICATESKEYWORDSCOUNT 5

#define SPECIAL_ID_INVALID -1
#define SPECIAL_ID_ARTWORK_PROXY_MODEL 2
#define SPECIAL_ID_COMBINED_MODEL 3

#define INVALID_BATCH_ID 0

#endif // DEFINES


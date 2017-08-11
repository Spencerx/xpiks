/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2017 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef VERSION_H
#define VERSION_H

// About dialog
#define XPIKS_VERSION_SUFFIX tags

// update.json (values from 0..9)
#define XPIKS_MAJOR_VERSION 1
#define XPIKS_MINOR_VERSION 4
#define XPIKS_FIX_VERSION 2

#define XPIKS_VERSION_INT (XPIKS_MAJOR_VERSION*100 + XPIKS_MINOR_VERSION*10 + XPIKS_FIX_VERSION)

#ifdef QT_DEBUG
#define XPIKS_VERSION_STRING "1.4.2.dev"
#else
#define XPIKS_VERSION_STRING "1.4.2 tk"
#endif

#endif // VERSION_H


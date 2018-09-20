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

#define STRINGIZE_(x) #x
#define STRINGIZE(x) STRINGIZE_(x)

// 4*(avg word size 10 * avg keywords size 50)
#define MAX_PASTE_SIZE 2000

#define PREVIEWOFFSET 25
#define PREVIEWKEYWORDSCOUNT 5

#define DUPLICATEOFFSET 25
#define DUPLICATESKEYWORDSCOUNT 5

#define SPECIAL_ID_INVALID -1
#define SPECIAL_ID_ARTWORK_PROXY_MODEL 2
#define SPECIAL_ID_COMBINED_MODEL 3

#define INVALID_BATCH_ID 0

#endif // DEFINES


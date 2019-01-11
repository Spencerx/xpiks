/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef IWARNINGSSETTINGS_H
#define IWARNINGSSETTINGS_H

class QString;

namespace Warnings {
    class IWarningsSettings {
    public:
        virtual ~IWarningsSettings() {}

    public:
        virtual const QString &getAllowedFilenameCharacters() const = 0;
        virtual double getMinMegapixels() const = 0;
        virtual double getMaxImageFilesizeMB() const = 0;
        virtual double getMaxVideoFilesizeMB() const = 0;
        virtual double getMinVideoDurationSeconds() const = 0;
        virtual double getMaxVideoDurationSeconds() const = 0;
        virtual int getMinKeywordsCount() const = 0;
        virtual int getMaxKeywordsCount() const = 0;
        virtual int getMinWordsCount() const = 0;
        virtual int getMaxDescriptionLength() const = 0;
        virtual int getMaxTitleWords() const = 0;
    };
}

#endif // IWARNINGSSETTINGS_H

/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef IMETADATADUPLICATES_H
#define IMETADATADUPLICATES_H

#include <QString>

namespace Artworks {
    class BasicMetadataModel;
}

namespace SpellCheck {
    class IMetadataDuplicates {
    public:
        virtual ~IMetadataDuplicates() { }
        virtual Artworks::BasicMetadataModel &getBasicModel() = 0;
        virtual QString getDescriptionDuplicates() = 0;
        virtual QString getTitleDuplicates() = 0;
        virtual QString getKeywordsDuplicates() = 0;
    };
}

#endif // IMETADATADUPLICATES_H

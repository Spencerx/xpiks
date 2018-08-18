/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef ISPELLCHECKSERVICE_H
#define ISPELLCHECKSERVICE_H

#include <Common/flags.h>
#include <QStringList>

namespace Artworks {
    class IBasicModelSource;
}

namespace SpellCheck {
    class ISpellCheckService {
        virtual ~ISpellCheckService() { }
        virtual quint32 submitItems(std::vector<std::shared_ptr<Artworks::IBasicModelSource>> const &items,
                                       QStringList const &wordsToCheck) = 0;
        virtual void submitItem(std::shared_ptr<Artworks::IBasicModelSource> const &item) = 0;
    };
}

#endif // ISPELLCHECKSERVICE_H

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
    class BasicKeywordsModel;
    class ArtworksSnapshot;
    class ArtworkMetadata;
}

namespace SpellCheck {
    class IBasicModelSpellCheckService {
        virtual ~IBasicModelSpellCheckService() {}
        virtual void submitItem(Artworks::BasicKeywordsModel const &itemToCheck, Common::SpellCheckFlags flags) = 0;
    };

    class IArtworkSpellCheckService {
        virtual ~IArtworkSpellCheckService() {}
        virtual quint32 submitArtworks(Artworks::ArtworksSnapshot const &snapshot, QStringList const &wordsToCheck) = 0;
        virtual void submitArtwork(std::shared_ptr<Artworks::ArtworkMetadata> const &artwork) = 0;
    };
}

#endif // ISPELLCHECKSERVICE_H

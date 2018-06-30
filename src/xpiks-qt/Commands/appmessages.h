/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef APPMESSAGES_H
#define APPMESSAGES_H

#include <QString>
#include <QStringList>
#include <QVector>
#include "messages.h"
#include <Helpers/indiceshelper.h>
#include <Artworks/artworkssnapshot.h>
#include <Models/Editing/icurrenteditable.h>
#include <Common/types.h>

namespace Artworks {
    class BasicKeywordsModel;
    class VideoArtwork;
}

namespace Commands {
    class AppMessages: RegisteredMessages<
            MessageMap<>,
            MessageMap<size_t>,
            MessageMap<Helpers::IndicesRanges>,
            MessageMap<Artworks::ArtworksSnapshot>,
            MessageMap<std::shared_ptr<Models::ICurrentEditable>>,
            MessageMap<Artworks::BasicKeywordsModel*>,
            MessageMap<QString, QString, QStringList, bool>,
            MessageMap<Common::ID_t, size_t, QVector<int>>,
            MessageMap<Artworks::VideoArtwork*>
            >
    {
    public:
        enum MessageID {
            RegisterCurrentEditable,
            UpdateArtworks,
            BackupArtworks,
            SpellCheck,
            CopyToQuickBuffer,
            ZipArtworks,
            UploadArtworks,
            CreateThumbnail,
            ExportToCSV,
        };
    };
}

#endif // APPMESSAGES_H

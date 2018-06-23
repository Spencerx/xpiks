/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef KEYWORDEDITTEMPLATE_H
#define KEYWORDEDITTEMPLATE_H

#include "iartworkscommandtemplate.h"
#include <QString>
#include <vector>
#include "../UndoRedo/artworkmetadatabackup.h"
#include "../Common/flags.h"

namespace Commands {
    class KeywordEditTemplate: public IArtworksCommandTemplate
    {
    public:
        KeywordEditTemplate(Common::KeywordEditFlags editFlags,
                            int keywordIndex = -1,
                            const QString &nextValue = QString());

        // IArtworksCommandTemplate interface
    public:
        virtual void execute(Artworks::ArtworksSnapshot &snapshot) override;
        virtual void undo(Artworks::ArtworksSnapshot &) override;

    private:
        int m_KeywordIndex;
        Common::KeywordEditFlags m_EditFlags;
        QString m_NextValue;
    };
}

#endif // KEYWORDEDITTEMPLATE_H

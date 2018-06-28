/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SELECTEDARTWORKSCONSUMER_H
#define SELECTEDARTWORKSCONSUMER_H

#include <Artworks/iartworkssource.h>

namespace Models {
    class SelectedArtworksConsumer {
    public:
        SelectedArtworksConsumer(Artworks::IArtworksSource &selectedArtworksSource):
            m_SelectedArtworksSource(selectedArtworksSource)
        { }

    public:
        void pullArtworks() { setArtworks(m_SelectedArtworksSource.getArtworks()); }

    protected:
        virtual void setArtworks(Artworks::WeakArtworksSnapshot &snapshot) = 0;

    private:
        Artworks::IArtworksSource &m_SelectedArtworksSource;
    };
}

#endif // SELECTEDARTWORKSCONSUMER_H

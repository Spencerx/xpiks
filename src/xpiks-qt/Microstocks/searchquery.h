/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SEARCHQUERY_H
#define SEARCHQUERY_H

#include <QObject>
#include <QStringList>
#include "../Common/flags.h"

namespace Microstocks {
    class SearchQuery {
    private:
        enum QueryFlags {
            FlagAllImages = 1 << 0,
            FlagPhotos = 1 << 1,
            FlagVectors = 1 << 2,
            FlagIllustrations = 1 << 3,
            FlagVideos = 1 << 4,
            FlagFullSearch = 1 << 5
        };

    private:
        inline bool getAllImagesFlag() const { return Common::HasFlag(m_Flags, FlagAllImages); }
        inline bool getPhotosFlag() const { return Common::HasFlag(m_Flags, FlagPhotos); }
        inline bool getVectorsFlag() const { return Common::HasFlag(m_Flags, FlagVectors); }
        inline bool getIllustrationsFlag() const { return Common::HasFlag(m_Flags, FlagIllustrations); }
        inline bool getVideosFlag() const { return Common::HasFlag(m_Flags, FlagVideos); }
        inline bool getFullSearchFlag() const { return Common::HasFlag(m_Flags, FlagFullSearch); }

        inline void setAllImagesFlag(bool value) { Common::ApplyFlag(m_Flags, value, FlagAllImages); }
        inline void setPhotosFlag(bool value) { Common::ApplyFlag(m_Flags, value, FlagPhotos); }
        inline void setVectorsFlag(bool value) { Common::ApplyFlag(m_Flags, value, FlagVectors); }
        inline void setIllustrationsFlag(bool value) { Common::ApplyFlag(m_Flags, value, FlagIllustrations); }
        inline void setVideosFlag(bool value) { Common::ApplyFlag(m_Flags, value, FlagVideos); }
        inline void setFullSearchFlag(bool value) { Common::ApplyFlag(m_Flags, value, FlagFullSearch); }

    public:
        SearchQuery():
            m_Flags(0),
            m_PageIndex(0),
            m_PageSize(100)
        {}

        SearchQuery(const QString &searchTerm, int resultType, int maxResults):
            m_Flags(0),
            m_PageIndex(0),
            m_PageSize(maxResults)
        {
            // "All Images"
            // "Photos"
            // "Vectors"
            // "Illustrations"

            m_SearchTerms = searchTerm.split(QChar::Space, QString::SkipEmptyParts);

            switch (resultType) {
            case 0:
                setAllImagesFlag(true);
                break;
            case 1:
                setPhotosFlag(true);
                break;
            case 2:
                setVectorsFlag(true);
                break;
            case 3:
                setIllustrationsFlag(true);
                break;
            case 4:
                setVideosFlag(true);
                break;
            default:
                break;
            }
        }

    public:
        const QStringList &getSearchTerms() const { return m_SearchTerms; }
        int getPageSize() const { return m_PageSize; }
        int getPageIndex() const { return m_PageIndex; }
        bool getFullSearch() const { return getFullSearchFlag(); }
        bool getSearchAllImages() const { return getAllImagesFlag(); }
        bool getSearchPhotos() const { return getPhotosFlag(); }
        bool getSearchVectors() const { return getVectorsFlag(); }
        bool getSearchIllustrations() const { return getIllustrationsFlag(); }
        bool getSearchVideos() const { return getVideosFlag(); }

    private:
        QStringList m_SearchTerms;
        Common::flag_t m_Flags;
        int m_PageIndex;
        int m_PageSize;
    };
}

#endif // SEARCHQUERY_H

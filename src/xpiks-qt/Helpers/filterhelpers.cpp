/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "filterhelpers.h"
#include <QString>
#include "../Models/artworkmetadata.h"
#include "../Models/imageartwork.h"
#include "../Common/basickeywordsmodel.h"
#include "../Common/flags.h"
#include "../Common/defines.h"

namespace Helpers {
    bool fitsSpecialKeywords(const QString &searchTerm, Models::ArtworkMetadata *metadata) {
        bool hasMatch = false;

        const Models::ImageArtwork *image = dynamic_cast<const Models::ImageArtwork*>(metadata);
        if (image == NULL) { return hasMatch; }

        if (searchTerm == QLatin1String("x:modified")) {
            hasMatch = metadata->isModified();
        } else if (searchTerm == QLatin1String("x:empty")) {
            hasMatch = metadata->isEmpty();
        } else if (searchTerm == QLatin1String("x:selected")) {
            hasMatch = metadata->isSelected();
        } else if (searchTerm == QLatin1String("x:vector")) {
            hasMatch = image != NULL && image->hasVectorAttached();
        } else if (searchTerm == QLatin1String("x:image")) {
            hasMatch = image != NULL && !image->hasVectorAttached();
        }

        return hasMatch;
    }

    bool containsAnyPartsSearch(const QString &mainSearchTerm, Models::ArtworkMetadata *metadata, Common::SearchFlags searchFlags) {
        bool hasMatch = false;
        QStringList searchTerms;

        if (!Common::HasFlag(searchFlags, Common::SearchFlags::IncludeSpaces)) {
            searchTerms = mainSearchTerm.split(QChar::Space, QString::SkipEmptyParts);
        } else {
            searchTerms << mainSearchTerm;
        }

        const QString description = metadata->getDescription();
        const QString title = metadata->getTitle();
        const QString &filepath = metadata->getFilepath();

        const bool needToCheckDescription = Common::HasFlag(searchFlags, Common::SearchFlags::Description);
        const bool needToCheckTitle = Common::HasFlag(searchFlags, Common::SearchFlags::Title);
        const bool needToCheckSpecial = Common::HasFlag(searchFlags, Common::SearchFlags::ReservedTerms);
        const bool needToCheckFilepath = Common::HasFlag(searchFlags, Common::SearchFlags::Filepath);
        const bool caseSensitive = Common::HasFlag(searchFlags, Common::SearchFlags::CaseSensitive);
        Qt::CaseSensitivity caseSensitivity = caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive;

        int length = searchTerms.length();

        Common::BasicKeywordsModel *keywordsModel = metadata->getBasicModel();

        for (int i = 0; i < length; ++i) {
            const QString &searchTerm = searchTerms.at(i);

            if (needToCheckSpecial) {
                hasMatch = fitsSpecialKeywords(searchTerm, metadata);
            }

            if (!hasMatch && needToCheckDescription) {
                hasMatch = description.contains(searchTerm, caseSensitivity);
            }

            if (!hasMatch && needToCheckTitle) {
                hasMatch = title.contains(searchTerm, caseSensitivity);
            }

            if (!hasMatch && needToCheckFilepath) {
                hasMatch = filepath.contains(searchTerm, caseSensitivity);
            }

            if (hasMatch) { break; }
        }


        const bool needToCheckKeywords = Common::HasFlag(searchFlags, Common::SearchFlags::Keywords);

        if (!hasMatch && needToCheckKeywords) {
            for (int i = 0; i < length; ++i) {
                QString searchTerm = searchTerms[i];
                Common::SearchFlags keywordsFlags = Common::SearchFlags::Keywords;

                if ((searchTerm.length() > 1) && searchTerm[0] == QLatin1Char('!')) {
                    Common::SetFlag(keywordsFlags, Common::SearchFlags::WholeWords);
                    searchTerm.remove(0, 1);
                }

                if (caseSensitive) {
                    Common::SetFlag(keywordsFlags, Common::SearchFlags::CaseSensitive);
                }

                hasMatch = keywordsModel->containsKeyword(searchTerm, keywordsFlags);
                if (hasMatch) { break; }
            }
        }

        return hasMatch;
    }

    bool containsAllPartsSearch(const QString &mainSearchTerm, Models::ArtworkMetadata *metadata, Common::SearchFlags searchFlags) {
        bool hasMatch = false;
        QStringList searchTerms;

        if (!Common::HasFlag(searchFlags, Common::SearchFlags::IncludeSpaces)) {
            searchTerms = mainSearchTerm.split(QChar::Space, QString::SkipEmptyParts);
        } else {
            searchTerms << mainSearchTerm;
        }

        const QString description = metadata->getDescription();
        const QString title = metadata->getTitle();
        const QString &filepath = metadata->getFilepath();

        const bool needToCheckDescription = Common::HasFlag(searchFlags, Common::SearchFlags::Description);
        const bool needToCheckTitle = Common::HasFlag(searchFlags, Common::SearchFlags::Title);
        const bool needToCheckSpecial = Common::HasFlag(searchFlags, Common::SearchFlags::ReservedTerms);
        const bool needToCheckFilepath = Common::HasFlag(searchFlags, Common::SearchFlags::Filepath);
        const bool caseSensitive = Common::HasFlag(searchFlags, Common::SearchFlags::CaseSensitive);
        Qt::CaseSensitivity caseSensitivity = caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive;

        bool anyError = false;
        int length = searchTerms.length();

        Common::BasicKeywordsModel *keywordsModel = metadata->getBasicModel();

        for (int i = 0; i < length; ++i) {
            QString searchTerm = searchTerms[i];
            bool anyContains = false;

            if (needToCheckSpecial) {
                anyContains = fitsSpecialKeywords(searchTerm, metadata);
            }

            if (!anyContains && needToCheckDescription) {
                anyContains = description.contains(searchTerm, caseSensitivity);
            }

            if (!anyContains && needToCheckTitle) {
                anyContains = title.contains(searchTerm, caseSensitivity);
            }

            if (!anyContains && needToCheckFilepath) {
                anyContains = filepath.contains(searchTerm, caseSensitivity);
            }

            const bool needToCheckKeywords = Common::HasFlag(searchFlags, Common::SearchFlags::Keywords);

            if (!anyContains && needToCheckKeywords) {
                Common::SearchFlags keywordsFlags = Common::SearchFlags::Keywords;

                if ((searchTerm.length() > 1) && searchTerm[0] == QLatin1Char('!')) {
                    Common::SetFlag(keywordsFlags, Common::SearchFlags::WholeWords);
                    searchTerm.remove(0, 1);
                }

                if (caseSensitive) {
                    Common::SetFlag(keywordsFlags, Common::SearchFlags::CaseSensitive);
                }

                anyContains = keywordsModel->containsKeyword(searchTerm, keywordsFlags);
            }

            if (!anyContains) {
                anyError = true;
                break;
            }
        }

        hasMatch = !anyError;
        return hasMatch;
    }

    bool hasSearchMatch(const QString &searchTerm, Models::ArtworkMetadata *metadata, Common::SearchFlags searchFlags) {
        bool hasMatch = false;

        const bool searchUsingAnd = Common::HasFlag(searchFlags, Common::SearchFlags::AllTerms);
        LOG_CORE_TESTS << "Search using AND:" << searchUsingAnd;
        LOG_CORE_TESTS << "Case sensitive:" << Common::HasFlag(searchFlags, Common::SearchFlags::CaseSensitive);

        if (searchUsingAnd) {
            hasMatch = containsAllPartsSearch(searchTerm, metadata, searchFlags);
        } else {
            hasMatch = containsAnyPartsSearch(searchTerm, metadata, searchFlags);
        }

        return hasMatch;
    }

}

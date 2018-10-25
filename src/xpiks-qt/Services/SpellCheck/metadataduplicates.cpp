/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "metadataduplicates.h"

#include <vector>

#include <QStringList>

#include "Artworks/artworkmetadata.h"
#include "Artworks/basicmetadatamodel.h"
#include "Artworks/keyword.h"
#include "Common/defines.h"
#include "Helpers/stringhelper.h"
#include "Services/SpellCheck/spellcheckinfo.h"

namespace SpellCheck {
    BasicModelDuplicates::BasicModelDuplicates(Artworks::BasicMetadataModel &basicModel):
        m_BasicModel(basicModel)
    {
    }

    QString BasicModelDuplicates::getDescriptionDuplicates() {
        SpellCheck::SpellCheckInfo &spellCheckInfo = m_BasicModel.getSpellCheckInfo();

        QString description = m_BasicModel.getDescription();
        QString result;

        if (spellCheckInfo.anyDescriptionDuplicates()) {
            std::vector<int> hits;
            QString descriptionLower = description.toLower();

            Helpers::foreachWord(descriptionLower,
                                 [&spellCheckInfo](const QString &word) {
                return spellCheckInfo.hasDescriptionDuplicate(word);
            },
            [&hits](int start, int length, const QString &) {
                hits.push_back(start + length/2);
            });

            result = Helpers::getUnitedHitsString(description, hits, DUPLICATEOFFSET);
        } else {
            if (description.size() > PREVIEWOFFSET*2) {
                result = description.left(PREVIEWOFFSET*2) + " ...";
            } else {
                result = description;
            }
        }

        return result;
    }

    QString BasicModelDuplicates::getTitleDuplicates() {
        SpellCheck::SpellCheckInfo &spellCheckInfo = m_BasicModel.getSpellCheckInfo();

        QString title = m_BasicModel.getTitle();
        QString result;

        if (spellCheckInfo.anyTitleDuplicates()) {
            std::vector<int> hits;
            QString titleLower = title.toLower();

            Helpers::foreachWord(titleLower,
                                 [&spellCheckInfo](const QString &word) {
                return spellCheckInfo.hasTitleDuplicate(word);
            },
            [&hits](int start, int length, const QString &) {
                hits.push_back(start + length/2);
            });

            result = Helpers::getUnitedHitsString(title, hits, DUPLICATEOFFSET);
        } else {
            if (title.size() > PREVIEWOFFSET*2) {
                result = title.left(PREVIEWOFFSET*2) + " ...";
            } else {
                result = title;
            }
        }

        return result;
    }

    QString BasicModelDuplicates::getKeywordsDuplicates() {
        std::vector<Artworks::KeywordItem> duplicatedKeywords = m_BasicModel.retrieveDuplicatedKeywords();

        QString text;

        if (!duplicatedKeywords.empty()) {
            QStringList keywords;
            keywords.reserve((int)duplicatedKeywords.size());
            for (auto &item: duplicatedKeywords) {
                keywords.append(item.m_Word);
            }

            text = keywords.join(", ");
        } else {
            QStringList keywords = m_BasicModel.getKeywords();
            if (keywords.length() > DUPLICATESKEYWORDSCOUNT) {
                QStringList part = keywords.mid(0, DUPLICATESKEYWORDSCOUNT);
                text = part.join(", ") + " ...";
            } else {
                text = keywords.join(", ");
            }
        }

        return text;
    }

    ArtworkMetadataDuplicates::ArtworkMetadataDuplicates(const std::shared_ptr<Artworks::ArtworkMetadata> &artwork):
        BasicModelDuplicates(artwork->getBasicMetadataModel()),
        m_Artwork(artwork)
    {
    }
}

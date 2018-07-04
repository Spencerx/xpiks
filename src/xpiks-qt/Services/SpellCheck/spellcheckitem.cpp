/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "spellcheckitem.h"
#include <QStringList>
#include <Common/flags.h>
#include <Common/defines.h>
#include <Helpers/indiceshelper.h>
#include <Helpers/stringhelper.h>
#include <Artworks/artworkmetadata.h>
#include <Artworks/basicmetadatamodel.h>

/*
 * When spellcheck results are propagated back, they are updated in artwork
 * based on index of a keyword. So in order to distinguish keywords from title
 * and description, this workaround with "int_max" indices is used
 */
#define IMPOSSIBLE_TITLE_INDEX 100000
#define IMPOSSIBLE_DESCRIPTION_INDEX 200000

namespace SpellCheck {
    SpellCheckItem::SpellCheckItem(Artworks::BasicKeywordsModel *spellCheckable,
                                   Common::SpellCheckFlags spellCheckFlags,
                                   Common::WordAnalysisFlags wordAnalysisFlags):
        QObject(),
        m_BasicModel(spellCheckable),
        m_SpellCheckFlags(spellCheckFlags),
        m_WordAnalysisFlags(wordAnalysisFlags),
        m_NeedsSuggestions(false),
        m_OnlyOneKeyword(false)
    {
        Q_ASSERT(m_BasicModel != NULL);
        m_BasicModel->acquire();

        QObject::connect(this, &SpellCheck::SpellCheckItem::resultsReady,
                         m_BasicModel, &Artworks::BasicKeywordsModel::onSpellCheckRequestReady);

        std::function<bool (const QString &word)> alwaysTrue = [](const QString &) {return true; };

        if (Common::HasFlag(spellCheckFlags, Common::SpellCheckFlags::Keywords)) {
            QStringList keywords = spellCheckable->getKeywords();
            addWords(keywords, 0, alwaysTrue);
        }

        if (Common::HasFlag(spellCheckFlags, Common::SpellCheckFlags::Description)) {
            Artworks::BasicMetadataModel *metadataModel = dynamic_cast<Artworks::BasicMetadataModel*>(spellCheckable);
            if (metadataModel != nullptr) {
                QStringList descriptionWords = metadataModel->getDescriptionWords();
                addWords(descriptionWords, IMPOSSIBLE_DESCRIPTION_INDEX, alwaysTrue);
            }
        }

        if (Common::HasFlag(spellCheckFlags, Common::SpellCheckFlags::Title)) {
            Artworks::BasicMetadataModel *metadataModel = dynamic_cast<Artworks::BasicMetadataModel*>(spellCheckable);
            if (metadataModel != nullptr) {
                QStringList titleWords = metadataModel->getTitleWords();
                addWords(titleWords, IMPOSSIBLE_TITLE_INDEX, alwaysTrue);
            }
        }
    }

    SpellCheckItem::SpellCheckItem(Artworks::BasicKeywordsModel *spellCheckable,
                                   const QStringList &keywordsToCheck,
                                   Common::WordAnalysisFlags wordAnalysisFlags):
        QObject(),
        m_BasicModel(spellCheckable),
        m_SpellCheckFlags(Common::SpellCheckFlags::All),
        m_WordAnalysisFlags(wordAnalysisFlags),
        m_NeedsSuggestions(false),
        m_OnlyOneKeyword(false)
    {
        Q_ASSERT(m_BasicModel != NULL);
        m_BasicModel->acquire();

        QObject::connect(this, &SpellCheck::SpellCheckItem::resultsReady,
                         m_BasicModel, &Artworks::BasicKeywordsModel::onSpellCheckRequestReady);

        std::function<bool (const QString &word)> containsFunc = [&keywordsToCheck](const QString &word) {
                                                                     bool contains = false;

                                                                     for (auto &item: keywordsToCheck) {
                                                                         if (word.contains(item, Qt::CaseInsensitive)) {
                                                                             contains = true;
                                                                             break;
                                                                         }
                                                                     }

                                                                     return contains;
                                                                 };

        QStringList keywords = spellCheckable->getKeywords();
        addWords(keywords, 0, containsFunc);

        std::function<bool (const QString &word)> sameKeywordFunc = [&keywordsToCheck](const QString &word) {
                                                                        bool match = false;

                                                                        for (auto &item: keywordsToCheck) {
                                                                            if (QString::compare(word, item, Qt::CaseInsensitive) == 0) {
                                                                                match = true;
                                                                                break;
                                                                            }
                                                                        }

                                                                        return match;
                                                                    };

        Artworks::BasicMetadataModel *metadataModel = dynamic_cast<Artworks::BasicMetadataModel*>(m_BasicModel);
        if (metadataModel != nullptr) {
            QStringList descriptionWords = metadataModel->getDescriptionWords();
            addWords(descriptionWords, IMPOSSIBLE_DESCRIPTION_INDEX, sameKeywordFunc);

            QStringList titleWords = metadataModel->getTitleWords();
            addWords(titleWords, IMPOSSIBLE_TITLE_INDEX, sameKeywordFunc);
        }
    }

    SpellCheckItem::~SpellCheckItem() {
        LOG_INTEGRATION_TESTS << "Actually deleting SpellCheckItem";

        Q_ASSERT(m_BasicModel != nullptr);
        if (m_BasicModel->release()) {
            LOG_WARNING << "Item could have been removed";
        }
    }

    void SpellCheckItem::addWords(const QStringList &words, int startingIndex, const std::function<bool (const QString &word)> &pred) {
        int index = startingIndex;
        m_QueryItems.reserve(m_QueryItems.size() + words.size());

        foreach(const QString &word, words) {
            if (!word.contains(QChar::Space)) {
                if (pred(word)) {
                    std::shared_ptr<SpellCheckQueryItem> queryItem(new SpellCheckQueryItem(index, word));
                    m_QueryItems.emplace_back(queryItem);
                }
            } else {
                QStringList parts = word.split(QChar::Space, QString::SkipEmptyParts);
                foreach(const QString &part, parts) {
                    QString item = part.trimmed();

                    if (item.length() >= 2) {
                        if (pred(item)) {
                            std::shared_ptr<SpellCheckQueryItem> queryItem(new SpellCheckQueryItem(index, item));
                            m_QueryItems.emplace_back(queryItem);
                        }
                    }
                }
            }

            index++;
        }
    }

    /*virtual */
    void SpellCheckItem::submitSpellCheckResult() {
        const std::vector<std::shared_ptr<SpellCheckQueryItem> > &items = getQueries();

        // can be empty in case of clear command
        if (Common::HasFlag(m_SpellCheckFlags, Common::SpellCheckFlags::Keywords) && !items.empty()) {
            m_BasicModel->setKeywordsSpellCheckResults(items);
        }

        if (Common::HasFlag(m_SpellCheckFlags, Common::SpellCheckFlags::Description) ||
            Common::HasFlag(m_SpellCheckFlags, Common::SpellCheckFlags::Title)) {
            Artworks::BasicMetadataModel *metadataModel = dynamic_cast<Artworks::BasicMetadataModel*>(m_BasicModel);
            if (metadataModel != nullptr) {
                metadataModel->setSpellCheckResults(getHash(), m_SpellCheckFlags);
            }
        }

        int index = m_OnlyOneKeyword ? (int)items.front()->m_Index : -1;
        emit resultsReady(m_SpellCheckFlags, index);
    }

    void SpellCheckItem::accountResults() {
        for (auto &item: m_QueryItems) {
            m_SpellCheckResults.insert(item->m_Word,
                                       Common::WordAnalysisResult(item->m_Stem,
                                                                  item->m_IsCorrect,
                                                                  item->m_IsDuplicate));
        }
    }

    ArtworkSpellCheckItem::ArtworkSpellCheckItem(Artworks::ArtworkMetadata *artwork,
                                                 Common::SpellCheckFlags spellCheckFlags,
                                                 Common::WordAnalysisFlags wordAnalysisFlags):
        SpellCheckItem(artwork->getBasicModel(), spellCheckFlags, wordAnalysisFlags)
    {
        m_Artwork = artwork;
    }

    ArtworkSpellCheckItem::ArtworkSpellCheckItem(Artworks::ArtworkMetadata *artwork,
                                                 const QStringList &keywordsToCheck,
                                                 Common::WordAnalysisFlags wordAnalysisFlags) :
        SpellCheckItem(artwork->getBasicModel(), keywordsToCheck, wordAnalysisFlags)
    {
        m_Artwork = artwork;
    }
}

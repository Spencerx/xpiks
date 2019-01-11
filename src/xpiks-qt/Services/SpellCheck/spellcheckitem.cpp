/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "spellcheckitem.h"

#include <QChar>
#include <QStringList>
#include <Qt>
#include <QtDebug>
#include <QtGlobal>

#include "Artworks/basickeywordsmodel.h"
#include "Artworks/basicmetadatamodel.h"
#include "Artworks/ibasicmodelsource.h"
#include "Common/flags.h"
#include "Common/logging.h"
#include "Common/wordanalysisresult.h"

/*
 * When spellcheck results are propagated back, they are updated in artwork
 * based on index of a keyword. So in order to distinguish keywords from title
 * and description, this workaround with "int_max" indices is used
 */
#define IMPOSSIBLE_TITLE_INDEX 100000
#define IMPOSSIBLE_DESCRIPTION_INDEX 200000

namespace SpellCheck {
    SpellCheckItem::SpellCheckItem(std::shared_ptr<Artworks::IBasicModelSource> const &basicModelSource,
                                   Common::SpellCheckFlags spellCheckFlags,
                                   Common::WordAnalysisFlags wordAnalysisFlags):
        QObject(),
        m_BasicModelSource(basicModelSource),
        m_WordAnalysisFlags(wordAnalysisFlags),
        m_SpellCheckFlags(spellCheckFlags),
        m_NeedsSuggestions(false),
        m_OnlyOneKeyword(false)
    {
        auto &basicModel = m_BasicModelSource->getBasicModel();

        QObject::connect(this, &SpellCheck::SpellCheckItem::resultsReady,
                         &basicModel, &Artworks::BasicKeywordsModel::onSpellCheckRequestReady);

        std::function<bool (const QString &word)> alwaysTrue = [](const QString &) {return true; };

        if (Common::HasFlag(spellCheckFlags, Common::SpellCheckFlags::Keywords)) {
            QStringList keywords = basicModel.getKeywords();
            addWords(keywords, 0, alwaysTrue);
        }

        if (Common::HasFlag(spellCheckFlags, Common::SpellCheckFlags::Description)) {
            Artworks::BasicMetadataModel *metadataModel = dynamic_cast<Artworks::BasicMetadataModel*>(&basicModel);
            if (metadataModel != nullptr) {
                QStringList descriptionWords = metadataModel->getDescriptionWords();
                addWords(descriptionWords, IMPOSSIBLE_DESCRIPTION_INDEX, alwaysTrue);
            }
        }

        if (Common::HasFlag(spellCheckFlags, Common::SpellCheckFlags::Title)) {
            Artworks::BasicMetadataModel *metadataModel = dynamic_cast<Artworks::BasicMetadataModel*>(&basicModel);
            if (metadataModel != nullptr) {
                QStringList titleWords = metadataModel->getTitleWords();
                addWords(titleWords, IMPOSSIBLE_TITLE_INDEX, alwaysTrue);
            }
        }
    }

    SpellCheckItem::SpellCheckItem(std::shared_ptr<Artworks::IBasicModelSource> const &basicModelSource,
                                   const QStringList &keywordsToCheck,
                                   Common::WordAnalysisFlags wordAnalysisFlags):
        QObject(),
        m_BasicModelSource(basicModelSource),
        m_WordAnalysisFlags(wordAnalysisFlags),
        m_SpellCheckFlags(Common::SpellCheckFlags::All),
        m_NeedsSuggestions(false),
        m_OnlyOneKeyword(false)
    {
        auto &basicModel = m_BasicModelSource->getBasicModel();
        QObject::connect(this, &SpellCheck::SpellCheckItem::resultsReady,
                         &basicModel, &Artworks::BasicKeywordsModel::onSpellCheckRequestReady);

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

        QStringList keywords = basicModel.getKeywords();
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

        Artworks::BasicMetadataModel *metadataModel = dynamic_cast<Artworks::BasicMetadataModel*>(&basicModel);
        if (metadataModel != nullptr) {
            QStringList descriptionWords = metadataModel->getDescriptionWords();
            addWords(descriptionWords, IMPOSSIBLE_DESCRIPTION_INDEX, sameKeywordFunc);

            QStringList titleWords = metadataModel->getTitleWords();
            addWords(titleWords, IMPOSSIBLE_TITLE_INDEX, sameKeywordFunc);
        }
    }

    SpellCheckItem::~SpellCheckItem() {
        LOG_VERBOSE << "Actually deleting SpellCheckItem";
    }

    void SpellCheckItem::addWords(const QStringList &words, int startingIndex, const std::function<bool (const QString &word)> &pred) {
        int index = startingIndex;
        m_QueryItems.reserve(m_QueryItems.size() + words.size());

        foreach(const QString &word, words) {
            if (!word.contains(QChar::Space)) {
                if (pred(word)) {
                    auto queryItem = std::make_shared<SpellCheckQueryItem>(index, word);
                    m_QueryItems.emplace_back(queryItem);
                }
            } else {
                QStringList parts = word.split(QChar::Space, QString::SkipEmptyParts);
                foreach(const QString &part, parts) {
                    QString item = part.trimmed();

                    if (item.length() >= 2) {
                        if (pred(item)) {
                            auto queryItem = std::make_shared<SpellCheckQueryItem>(index, item);
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
        auto &basicModel = m_BasicModelSource->getBasicModel();
        const std::vector<std::shared_ptr<SpellCheckQueryItem> > &items = getQueries();

        // can be empty in case of clear command
        if (Common::HasFlag(m_SpellCheckFlags, Common::SpellCheckFlags::Keywords) && !items.empty()) {
            basicModel.setKeywordsSpellCheckResults(items);
        }

        if (Common::HasFlag(m_SpellCheckFlags, Common::SpellCheckFlags::Description) ||
            Common::HasFlag(m_SpellCheckFlags, Common::SpellCheckFlags::Title)) {
            Artworks::BasicMetadataModel *metadataModel = dynamic_cast<Artworks::BasicMetadataModel*>(&basicModel);
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
}

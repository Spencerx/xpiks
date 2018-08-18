/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "spellsuggestionstarget.h"
#include "ispellcheckable.h"
#include "spellsuggestionsitem.h"
#include "ispellcheckservice.h"
#include <Common/logging.h>
#include <Artworks/artworkssnapshot.h>
#include <Services/iartworksupdater.h>

namespace SpellCheck {
    typedef std::vector<std::shared_ptr<SpellSuggestionsItem> > SuggestionsVector;

    void addMisspelledKeywords(ISpellCheckable *item,
                               std::vector<Artworks::KeywordItem> const &keywords,
                               SuggestionsVector &requests) {
        requests.reserve(requests.size() + keywords.size());
        for (auto &keywordItem: keywords) {
            if (!keywordItem.isPartOfAKeyword()) {
                requests.emplace_back(
                            std::make_shared<KeywordSpellSuggestions>(
                                keywordItem.m_Word, keywordItem.m_Index, item));
            } else {
                requests.emplace_back(
                            std::make_shared<KeywordSpellSuggestions>(
                                keywordItem.m_Word, keywordItem.m_Index, keywordItem.m_OriginKeyword, item));
            }
        }

        LOG_DEBUG << keywords.size() << "keywords requests";
    }

    void addMisspelledTitle(ISpellCheckable *item, const QStringList &words, SuggestionsVector &requests) {
        requests.reserve(requests.size() + words.size());

        for (auto &word: words) {
            requests.emplace_back(std::make_shared<TitleSpellSuggestions>(word, item));
        }

        LOG_DEBUG << words.size() << "title requests";
    }

    void addMisspelledDescription(ISpellCheckable *item, const QStringList &words, SuggestionsVector &requests) {
        requests.reserve(requests.size() + words.size());

        for (auto &word: words) {
            requests.emplace_back(std::make_shared<DescriptionSpellSuggestions>(word, item));
        }

        LOG_DEBUG << words.size() << "description requests";
    }

    SuggestionsVector createSuggestionsRequests(Artworks::ArtworksSnapshot const &snapshot, Common::SpellCheckFlags flags) {
        SuggestionsVector requests;

        for (auto &artwork: snapshot.getRawData()) {
            if (Common::HasFlag(flags, Common::SpellCheckFlags::Keywords)) {
                auto keywords = artwork->retrieveMisspelledKeywords();
                addMisspelledKeywords(artwork, keywords, requests);
            }

            if (Common::HasFlag(flags, Common::SpellCheckFlags::Title)) {
                auto words = artwork->retrieveMisspelledTitleWords();
                addMisspelledTitle(artwork, words, requests);
            }

            if (Common::HasFlag(flags, Common::SpellCheckFlags::Description)) {
                auto words = artwork->retrieveMisspelledDescriptionWords();
                addMisspelledDescription(artwork, words, requests);
            }
        }

        return requests;
    }

    SuggestionsVector createSuggestionsRequests(Artworks::BasicMetadataModel *basicModel, Common::SpellCheckFlags flags) {
        SuggestionsVector requests;

        if (Common::HasFlag(flags, Common::SpellCheckFlags::Keywords)) {
            auto keywords = basicModel->retrieveMisspelledKeywords();
            addMisspelledKeywords(basicModel, keywords, requests);
        }

        if (Common::HasFlag(flags, Common::SpellCheckFlags::Title)) {
            auto words = basicModel->retrieveMisspelledTitleWords();
            addMisspelledTitle(basicModel, words, requests);
        }

        if (Common::HasFlag(flags, Common::SpellCheckFlags::Description)) {
            auto words = basicModel->retrieveMisspelledDescriptionWords();
            addMisspelledDescription(basicModel, words, requests);
        }

        return requests;
    }

    std::vector<std::shared_ptr<SpellSuggestionsItem> > BasicModelSuggestionTarget::generateSuggestionItems(Common::SpellCheckFlags flags) {
        return createSuggestionsRequests(m_BasicModel, flags);
    }

    void BasicModelSuggestionTarget::afterReplaceCallback() {
        LOG_DEBUG << "#";
        m_BasicModel.afterReplaceCallback();
        m_SpellCheckService.submitItem(m_BasicModel, Common::SpellCheckFlags::All);
    }

    ArtworksSuggestionTarget::ArtworksSuggestionTarget(Artworks::ArtworksSnapshot &snapshot,
                                                       IArtworkSpellCheckService &spellCheckService,
                                                       Services::IArtworksUpdater &artworksUpdater):
        m_Snapshot(std::move(snapshot)),
        m_SpellCheckService(spellCheckService),
        m_ArtworksUpdater(artworksUpdater)
    {
    }

    std::vector<std::shared_ptr<SpellSuggestionsItem>> ArtworksSuggestionTarget::generateSuggestionItems(Common::SpellCheckFlags flags) {
        return createSuggestionsRequests(m_Snapshot, flags);
    }

    void ArtworksSuggestionTarget::afterReplaceCallback() {
        LOG_DEBUG << "#";
        for (auto &artwork: m_Snapshot.getRawData()) {
            artwork->afterReplaceCallback();
        }

        m_SpellCheckService.submitArtworks(m_Snapshot, QStringList());
        m_ArtworksUpdater.updateArtworks(m_Snapshot);
    }

    size_t ArtworksSuggestionTarget::size() const {
        return m_Snapshot.size();
    }
}

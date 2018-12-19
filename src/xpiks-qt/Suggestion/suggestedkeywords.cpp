/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "suggestedkeywords.h"

#include <map>
#include <functional>

#include "Common/logging.h"

namespace Suggestion {
    void calculateBounds(int selectedCount, int &lowerBound, int &upperBound) {
        if (selectedCount <= 2) {
            lowerBound = 1;
            upperBound = qMax(selectedCount, 1);
        } else if (selectedCount <= 4) {
            lowerBound = 2;
            upperBound = 3;
        } else if (selectedCount <= 5) {
            lowerBound = 2;
            upperBound = 3;
        } else if (selectedCount <= 9) {
            upperBound = selectedCount / 2;
            lowerBound = upperBound - 1;
        } else if (selectedCount <= 15) {
            upperBound = selectedCount / 2 - 1;
            lowerBound = upperBound - 2;
        } else {
            upperBound = selectedCount / 2;
            lowerBound = upperBound - 2;
        }
    }

    SuggestedKeywords::SuggestedKeywords()
    {
    }

    void SuggestedKeywords::addKeywords(const QSet<QString> &keywords) {
        accountKeywords(keywords, +1);
    }

    void SuggestedKeywords::removeKeywords(const QSet<QString> &keywords) {
        accountKeywords(keywords, -1);
    }

    void SuggestedKeywords::setExistingKeywords(const QSet<QString> &existingKeywords) {
        m_ExistingKeywords = existingKeywords;
    }

    void SuggestedKeywords::reset() {
        m_ExistingKeywords.clear();
        m_KeywordsHash.clear();
    }

    void SuggestedKeywords::updateSuggestion(int artworksCount) {
        std::multimap<int, QString, std::greater<int>> selectedKeywords;
        int lowerThreshold, upperThreshold;
        calculateBounds(artworksCount, lowerThreshold, upperThreshold);

        auto hashIt = m_KeywordsHash.constBegin();
        auto hashItEnd = m_KeywordsHash.constEnd();

        for (; hashIt != hashItEnd; ++hashIt) {
            selectedKeywords.insert({hashIt.value(), hashIt.key()});
        }

        int maxSuggested = 35 + (qrand() % 10);
        int maxUpperBound = 40 + (qrand() % 5);
        int maxOthers = 35 + (qrand() % 10);

        m_SuggestedKeywords.reserve(maxSuggested);
        m_OtherKeywords.reserve(maxOthers);

        bool canAddToSuggested, canAddToOthers;
        const bool isOnlyOneArtwork = (artworksCount == 1);

        auto itEnd = selectedKeywords.cend();
        for (auto it = selectedKeywords.cbegin(); it != itEnd; ++it) {
            int frequency = it->first;
            const QString &frequentKeyword = it->second;

            if (frequency == 0) { continue; }
            if (m_ExistingKeywords.contains(frequentKeyword.toLower())) {
                LOG_DEBUG << "Skipping existing keyword" << frequentKeyword;
                continue;
            }

            int suggestedCount = m_SuggestedKeywords.length();

            canAddToSuggested = (frequency >= upperThreshold) && (suggestedCount <= maxUpperBound);
            canAddToOthers = frequency >= lowerThreshold;

            if (isOnlyOneArtwork || canAddToSuggested ||
                    (canAddToOthers && (suggestedCount <= maxSuggested))) {
                m_SuggestedKeywords.append(frequentKeyword);
            } else if (canAddToOthers || (m_OtherKeywords.length() <= maxOthers)) {
                m_OtherKeywords.append(frequentKeyword);

                if (m_OtherKeywords.length() > maxOthers) {
                    break;
                }
            }
        }
    }

    void SuggestedKeywords::accountKeywords(const QSet<QString> &keywords, int sign) {
        for (const QString &keyword: keywords) {
            if (m_KeywordsHash.contains(keyword)) {
                m_KeywordsHash[keyword] += sign;
            } else {
                m_KeywordsHash.insert(keyword, 1);
            }
        }
    }
}

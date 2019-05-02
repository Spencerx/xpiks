/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "deletekeywordsviewmodel.h"

#include <cstddef>
#include <initializer_list>
#include <utility>
#include <vector>

#include <QHash>
#include <QMap>
#include <QMultiMap>
#include <QQmlEngine>
#include <QSet>
#include <QtDebug>
#include <QtGlobal>

#include "Artworks/artworkmetadata.h"
#include "Artworks/basickeywordsmodel.h"
#include "Artworks/basicmodelsource.h"
#include "Commands/Base/compositecommandtemplate.h"
#include "Commands/Base/icommandtemplate.h"
#include "Commands/Base/templatedcommand.h"
#include "Commands/Editing/clearactionmodeltemplate.h"
#include "Commands/Editing/deletekeywordstemplate.h"
#include "Commands/Editing/modifyartworkscommand.h"
#include "Commands/artworksupdatetemplate.h"
#include "Commands/Services/sendmessagecommand.h"
#include "Common/logging.h"
#include "KeywordsPresets/ipresetsmanager.h"
#include "Models/Artworks/artworksviewmodel.h"

namespace Artworks {
    class ArtworkElement;
}

namespace Models {
    DeleteKeywordsViewModel::DeleteKeywordsViewModel(Services::IArtworksUpdater &artworksUpdater,
                                                     KeywordsPresets::IPresetsManager &presetsManager,
                                                     QObject *parent):
        Models::ArtworksViewModel(parent),
        m_PresetsManager(presetsManager),
        m_ArtworksUpdater(artworksUpdater),
        m_CaseSensitive(false)
    {
    }

    void DeleteKeywordsViewModel::setArtworks(Artworks::ArtworksSnapshot const &artworks) {
        LOG_DEBUG << "#";
        ArtworksViewModel::setArtworks(artworks);
        recombineKeywords();
        sendMessage(Connectivity::EventType::SetupDeleteKeywords);
    }

    bool DeleteKeywordsViewModel::removeUnavailableItems() {
        LOG_DEBUG << "#";
        bool anyRemoved = ArtworksViewModel::removeUnavailableItems();

        if (anyRemoved) {
            if (!isEmpty()) {
                recombineKeywords();
            }
        }

        return anyRemoved;
    }

    std::shared_ptr<Commands::ICommand> DeleteKeywordsViewModel::getActionCommand(bool yesno) {
        LOG_INFO << "keywords to delete:" << m_KeywordsToDeleteModel.getKeywordsCount();

        if (m_KeywordsToDeleteModel.getKeywordsCount() > 0 && yesno) {
            auto keywordsList = m_KeywordsToDeleteModel.getKeywords();
            if (!m_CaseSensitive) {
                for (auto &keyword: keywordsList) {
                    keyword = keyword.toLower();
                }
            }
            auto keywordsSet = keywordsList.toSet();
            auto snapshot = createSnapshot();

            using namespace Commands;
            using ArtworksTemplate = Commands::ICommandTemplate<Artworks::ArtworksSnapshot>;
            using ArtworksTemplateComposite = Commands::CompositeCommandTemplate<Artworks::ArtworksSnapshot>;

            return std::make_shared<ModifyArtworksCommand>(
                        std::move(snapshot),
                        std::make_shared<ArtworksTemplateComposite>(
                            std::initializer_list<std::shared_ptr<ArtworksTemplate>>{
                                std::make_shared<DeleteKeywordsTemplate>(
                                keywordsSet, m_CaseSensitive),
                                std::make_shared<Commands::ClearActionModelTemplate>(*this),
                                std::make_shared<Commands::ArtworksSnapshotUpdateTemplate>(m_ArtworksUpdater),
                                std::make_shared<Commands::SendMessageCommand<Connectivity::EventType>>(
                                *this, Connectivity::EventType::DeleteKeywords)}));
        } else {
            using TemplatedSnapshotCommand = Commands::TemplatedCommand<Artworks::ArtworksSnapshot>;
            return std::make_shared<TemplatedSnapshotCommand>(
                        Artworks::ArtworksSnapshot(),
                        std::make_shared<Commands::ClearActionModelTemplate>(*this));
        }
    }

    void DeleteKeywordsViewModel::resetModel() {
        LOG_DEBUG << "#";
        ArtworksViewModel::resetModel();

        m_CommonKeywordsModel.clearKeywords();
        m_KeywordsToDeleteModel.clearKeywords();

        setCaseSensitive(false);
    }

#if defined(UI_TESTS) || defined(INTEGRATION_TESTS)
    void DeleteKeywordsViewModel::clearModel() {
        m_CommonKeywordsModel.clearKeywords();
        m_KeywordsToDeleteModel.clearKeywords();
    }

    void DeleteKeywordsViewModel::appendCommonKeyword(const QString &keyword) {
        m_CommonKeywordsModel.appendKeyword(keyword);
        emit commonKeywordsCountChanged();
    }
#endif

    bool DeleteKeywordsViewModel::doRemoveSelectedArtworks() {
        bool anyRemoved = ArtworksViewModel::doRemoveSelectedArtworks();

        LOG_INFO << "Any removed:" << anyRemoved;

        if (anyRemoved) {
            if (!isEmpty()) {
                recombineKeywords();
            }
        }

        return anyRemoved;
    }

    QObject *DeleteKeywordsViewModel::getCommonKeywordsObject() {
        QObject *item = &m_CommonKeywordsModel;
        QQmlEngine::setObjectOwnership(item, QQmlEngine::CppOwnership);
        return item;
    }

    QObject *DeleteKeywordsViewModel::getKeywordsToDeleteObject() {
        QObject *item = &m_KeywordsToDeleteModel;
        QQmlEngine::setObjectOwnership(item, QQmlEngine::CppOwnership);
        return item;
    }

    void DeleteKeywordsViewModel::removeKeywordToDeleteAt(int keywordIndex) {
        LOG_DEBUG << "#";
        QString keyword;
        if (m_KeywordsToDeleteModel.removeKeywordAt(keywordIndex, keyword)) {
            emit keywordsToDeleteCountChanged();
        }
    }

    void DeleteKeywordsViewModel::removeLastKeywordToDelete() {
        LOG_DEBUG << "#";
        removeKeywordToDeleteAt(getKeywordsToDeleteCount() - 1);
    }

    void DeleteKeywordsViewModel::clearKeywordsToDelete() {
        LOG_DEBUG << "#";
        if (m_KeywordsToDeleteModel.clearKeywords()) {
            emit keywordsToDeleteCountChanged();
            submitForSpellCheck();
        }
    }

    QString DeleteKeywordsViewModel::removeCommonKeywordAt(int keywordIndex) {
        LOG_DEBUG << "index:" << keywordIndex;
        QString keyword;
        if (m_CommonKeywordsModel.removeKeywordAt(keywordIndex, keyword)) {
            emit commonKeywordsCountChanged();
            submitForSpellCheck();
        }

        return keyword;
    }

    void DeleteKeywordsViewModel::appendKeywordToDelete(const QString &keyword) {
        LOG_INFO << keyword;
        if (m_KeywordsToDeleteModel.appendKeyword(keyword)) {
            emit keywordsToDeleteCountChanged();
            submitForSpellCheck();
        }
    }

    void DeleteKeywordsViewModel::pasteKeywordsToDelete(const QStringList &keywords) {
        LOG_INFO << keywords.length() << "keyword(s)" << "|" << keywords;
        if (m_KeywordsToDeleteModel.appendKeywords(keywords) > 0) {
            emit keywordsToDeleteCountChanged();
            submitForSpellCheck();
        }
    }

    bool DeleteKeywordsViewModel::addPreset(KeywordsPresets::ID_t presetID) {
        bool success = false;
        LOG_INFO << "preset" << presetID;
        QStringList keywords;

        if (m_PresetsManager.tryGetPreset(presetID, keywords)) {
            if (m_KeywordsToDeleteModel.appendKeywords(keywords) > 0) {
                emit keywordsToDeleteCountChanged();
                submitForSpellCheck();
            }
        }

        return success;
    }

    void DeleteKeywordsViewModel::recombineKeywords() {
        LOG_DEBUG << "#";
        QHash<QString, int> keywordsHash;
        fillKeywordsHash(keywordsHash);
        LOG_INFO << "Found" << keywordsHash.size() << "keyword(s)";

        QMultiMap<int, QString> selectedKeywords;

        auto hashIt = keywordsHash.constBegin();
        auto hashItEnd = keywordsHash.constEnd();

        for (; hashIt != hashItEnd; ++hashIt) {
            selectedKeywords.insert(hashIt.value(), hashIt.key());
        }

        auto it = selectedKeywords.constEnd();
        auto itBegin = selectedKeywords.constBegin();

        QStringList commonKeywords;
        commonKeywords.reserve(50);

        int maxSize = 40 + qrand()%10;

        while (it != itBegin) {
            --it;

            int frequency = it.key();
            if (frequency == 0) { continue; }

            const QString &frequentKeyword = it.value();

            commonKeywords.append(frequentKeyword);
            if (commonKeywords.size() > maxSize) { break; }
        }

        LOG_INFO << "Found" << commonKeywords.size() << "common keywords";
        m_CommonKeywordsModel.setKeywords(commonKeywords);
        emit commonKeywordsCountChanged();
    }

    void DeleteKeywordsViewModel::fillKeywordsHash(QHash<QString, int> &keywordsHash) {
        LOG_DEBUG << "#";
        processArtworks([](std::shared_ptr<Artworks::ArtworkElement> const &) { return true; },
        [&keywordsHash](size_t, std::shared_ptr<Artworks::ArtworkMetadata> const &artwork) {
            const auto &keywords = artwork->getKeywords();

            for (auto &keyword: keywords) {
                if (keywordsHash.contains(keyword)) {
                    keywordsHash[keyword]++;
                } else {
                    keywordsHash.insert(keyword, 1);
                }
            }
        });
    }

    void DeleteKeywordsViewModel::submitForSpellCheck() {
        sendMessage(
                    BasicSpellCheckMessageType(
                        std::make_shared<Artworks::BasicModelSource>(m_KeywordsToDeleteModel)));
    }
}

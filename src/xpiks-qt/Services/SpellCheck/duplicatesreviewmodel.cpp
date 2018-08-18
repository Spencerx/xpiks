/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "duplicatesreviewmodel.h"
#include <QQuickTextDocument>
#include <QFileInfo>
#include "imetadataduplicates.h"
#include "metadataduplicates.h"
#include <Services/SpellCheck/duplicateshighlighter.h>
#include <Helpers/cpphelpers.h>
#include <Artworks/keyword.h>
#include <Artworks/videoartwork.h>
#include <Artworks/imageartwork.h>
#include <Common/defines.h>
#include <Common/logging.h>

namespace SpellCheck {
    DuplicatesReviewModel::DuplicatesReviewModel(QMLExtensions::ColorsModel &colorsModel):
        m_ColorsModel(colorsModel)
    {
    }

    void DuplicatesReviewModel::setupModel(Artworks::BasicMetadataModel &basicModel) {
        Q_ASSERT(m_DuplicatesList.empty());
        beginResetModel();
        {
            m_DuplicatesList.emplace_back(std::make_shared<BasicModelDuplicates>(basicModel));
        }
        endResetModel();
    }

    void DuplicatesReviewModel::setupModel(const Artworks::ArtworksSnapshot &snapshot) {
        Q_ASSERT(m_DuplicatesList.empty());
        decltype(m_DuplicatesList) duplicates;
        const size_t size = snapshot.size();
        duplicates.reserve(size);
        for (auto &artwork: snapshot.getRawData()) {
            duplicates.emplace_back(std::make_shared<ArtworkMetadataDuplicates>(artwork));
        }

        beginResetModel();
        {
            m_DuplicatesList.swap(duplicates);
        }
        endResetModel();
    }

    void DuplicatesReviewModel::clearDuplicates() {
        beginResetModel();
        {
            m_DuplicatesList.clear();
        }
        endResetModel();
    }

    void DuplicatesReviewModel::initTitleHighlighting(int index, QQuickTextDocument *document) {
#if !defined(CORE_TESTS) && !defined(INTEGRATION_TESTS)
        if ((index < 0) || (index >= (int)m_DuplicatesList.size())) { return; }

        auto &item = m_DuplicatesList.at(index);
        auto &basicModel = item->getBasicModel();
        SpellCheck::SpellCheckInfo &spellCheckInfo = basicModel.getSpellCheckInfo();

        SpellCheck::DuplicatesHighlighter *highlighter = new SpellCheck::DuplicatesHighlighter(
                    document->textDocument(),
                    m_ColorsModel,
                    &spellCheckInfo.getTitleErrors());

        QObject::connect(&basicModel, &Artworks::BasicMetadataModel::hasDuplicatesChanged,
                         highlighter, &DuplicatesHighlighter::rehighlight);

        //QObject::connect(this, &DuplicatesReviewModel::rehighlightRequired,
        //                 highlighter, &DuplicatesHighlighter::rehighlight);

        Q_UNUSED(highlighter);
#else
        Q_UNUSED(index);
        Q_UNUSED(document);
#endif
    }

    void DuplicatesReviewModel::initDescriptionHighlighting(int index, QQuickTextDocument *document) {
#if !defined(CORE_TESTS) && !defined(INTEGRATION_TESTS)
        if ((index < 0) || (index >= (int)m_DuplicatesList.size())) { return; }

        auto &item = m_DuplicatesList.at(index);
        auto &basicModel = item->getBasicModel();
        SpellCheck::SpellCheckInfo &spellCheckInfo = basicModel.getSpellCheckInfo();

        SpellCheck::DuplicatesHighlighter *highlighter = new SpellCheck::DuplicatesHighlighter(
                    document->textDocument(),
                    m_ColorsModel,
                    &spellCheckInfo.getDescriptionErrors());

        QObject::connect(&basicModel, &Artworks::BasicMetadataModel::hasDuplicatesChanged,
                         highlighter, &DuplicatesHighlighter::rehighlight);

        //QObject::connect(this, &DuplicatesReviewModel::rehighlightRequired,
        //                 highlighter, &DuplicatesHighlighter::rehighlight);

        Q_UNUSED(highlighter);
#else
        Q_UNUSED(index);
        Q_UNUSED(document);
#endif
    }

    void DuplicatesReviewModel::initKeywordsHighlighting(int index, QQuickTextDocument *document) {
#if !defined(CORE_TESTS) && !defined(INTEGRATION_TESTS)
        if ((index < 0) || (index >= (int)m_DuplicatesList.size())) { return; }

        auto &item = m_DuplicatesList.at(index);
        auto &basicModel = item->getBasicModel();

        bool hasKeywordsDuplicates = basicModel.hasKeywordsDuplicates();

        SpellCheck::DuplicatesHighlighter *highlighter = new SpellCheck::DuplicatesHighlighter(
                    document->textDocument(),
                    m_ColorsModel,
                    nullptr, // highlight all words
                    hasKeywordsDuplicates);

        QObject::connect(&basicModel, &Artworks::BasicMetadataModel::hasDuplicatesChanged,
                         highlighter, &DuplicatesHighlighter::keywordsDuplicatesChanged);

        Q_UNUSED(highlighter);
#else
        Q_UNUSED(index);
        Q_UNUSED(document);
#endif
    }

    QString DuplicatesReviewModel::getTitleDuplicates(int index) {
        LOG_FOR_DEBUG << index;
        if ((index < 0) || (index >= (int)m_DuplicatesList.size())) { return QString(); }

        auto &item = m_DuplicatesList.at(index);
        return item->getTitleDuplicates();
    }

    QString DuplicatesReviewModel::getDescriptionDuplicates(int index) {
        LOG_FOR_DEBUG << index;
        if ((index < 0) || (index >= (int)m_DuplicatesList.size())) { return QString(); }

        auto &item = m_DuplicatesList.at(index);
        return item->getDescriptionDuplicates();
    }

    QString DuplicatesReviewModel::getKeywordsDuplicates(int index) {
        LOG_FOR_DEBUG << index;
        if ((index < 0) || (index >= (int)m_DuplicatesList.size())) { return QString(); }

        auto &item = m_DuplicatesList.at(index);
        return item->getKeywordsDuplicates();
    }

    void DuplicatesReviewModel::clearModel() {
        clearDuplicates();
        m_PendingUpdates.clear();
    }

    void DuplicatesReviewModel::processPendingUpdates() {
        LOG_DEBUG << "#";
        if (m_DuplicatesList.empty()) {
            m_PendingUpdates.clear();
            return;
        }

        QSet<size_t> indicesSet = m_PendingUpdates.toList().toSet();
        std::vector<int> indicesToUpdate;
        for (auto &item: m_DuplicatesList) {
            auto artworkItem = std::dynamic_pointer_cast<ArtworkMetadataDuplicates>(item);
            if (artworkItem == nullptr) { continue; }
            auto &artwork = artworkItem->getArtwork();
            if (indicesSet.contains(artwork->getLastKnownIndex())) {
                indicesToUpdate.push_back(artwork->getLastKnownIndex());
            }
        }

        LOG_INTEGR_TESTS_OR_DEBUG << QVector<int>::fromStdVector(indicesToUpdate);
        Helpers::IndicesRanges ranges(indicesToUpdate);
        auto roles = QVector<int>() << TriggerRole;

        for (auto &r: ranges.getRanges()) {
            QModelIndex indexFrom = this->index(r.first, 0);
            QModelIndex indexTo = this->index(r.second, 0);
            emit dataChanged(indexFrom, indexTo, roles);
        }

        m_PendingUpdates.clear();
    }

    int DuplicatesReviewModel::rowCount(const QModelIndex &parent) const {
        Q_UNUSED(parent);
        return (int)m_DuplicatesList.size();
    }

    QVariant DuplicatesReviewModel::data(const QModelIndex &index, int role) const {
        int row = index.row();
        if ((row < 0) || (row >= (int)m_DuplicatesList.size())) { return QVariant(); }

        auto &item = m_DuplicatesList.at(row);

        auto artworkItem = std::dynamic_pointer_cast<ArtworkMetadataDuplicates>(item);
        if (artworkItem == nullptr) {
            return QVariant();
        }

        switch (role) {
            case HasPathRole: {
                return true;
            }
            case PathRole: {
                return artworkItem->getArtwork()->getThumbnailPath();
            }
            case OriginalIndexRole: {
                return (int)artworkItem->getArtwork()->getLastKnownIndex();
            }
            case HasVectorAttachedRole: {
                auto image = std::dynamic_pointer_cast<Artworks::ImageArtwork>(artworkItem->getArtwork());
                return (image != nullptr) && image->hasVectorAttached();
            }
            case BaseFilenameRole:
                return artworkItem->getArtwork()->getBaseFilename();
            case IsVideoRole: {
                auto videoArtwork = std::dynamic_pointer_cast<Artworks::VideoArtwork>(artworkItem);
                return (videoArtwork != nullptr);
            }
            case TriggerRole: return QString();
            default: return QVariant();
        }
    }

    QHash<int, QByteArray> DuplicatesReviewModel::roleNames() const {
        QHash<int, QByteArray> roleNames = QAbstractListModel::roleNames();
        roleNames[HasPathRole] = "hasthumbnail";
        roleNames[PathRole] = "thumbpath";
        roleNames[OriginalIndexRole] = "originalIndex";
        roleNames[IsVideoRole] = "isvideo";
        roleNames[HasVectorAttachedRole] = "hasvectorattached";
        roleNames[BaseFilenameRole] = "basefilename";
        roleNames[TriggerRole] = "dtrigger";
        return roleNames;
    }

    void DuplicatesReviewModel::onDuplicatesCouldHaveChanged(size_t originalIndex) {
        LOG_DEBUG << originalIndex;
        m_PendingUpdates.push_back(originalIndex);
    }
}

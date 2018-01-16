/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "findandreplacemodel.h"
#include <QAbstractListModel>
#include "../Models/artworkmetadata.h"
#include "../Models/artitemsmodel.h"
#include "../Models/settingsmodel.h"
#include "../Commands/commandmanager.h"
#include "../Helpers/filterhelpers.h"
#include "../Models/filteredartitemsproxymodel.h"
#include "../Models/previewartworkelement.h"
#include "../Helpers/metadatahighlighter.h"
#include "../Commands/findandreplacecommand.h"
#include "../Common/defines.h"
#include "../Helpers/stringhelper.h"
#include "../Models/videoartwork.h"

QString searchFlagsToString(Common::SearchFlags flags) {
    QStringList items;
    items.reserve(10);

    if (Common::HasFlag(flags, Common::SearchFlags::CaseSensitive)) {
        items.append("CaseSensitive");
    }

    if (Common::HasFlag(flags, Common::SearchFlags::Description)) {
        items.append("Description");
    }

    if (Common::HasFlag(flags, Common::SearchFlags::Title)) {
        items.append("Title");
    }

    if (Common::HasFlag(flags, Common::SearchFlags::Keywords)) {
        items.append("Keywords");
    }

    if (Common::HasFlag(flags, Common::SearchFlags::WholeWords)) {
        items.append("WholeWords");
    }

    if (Common::HasFlag(flags, Common::SearchFlags::AllTerms)) {
        items.append("AllTerms");
    }

    return items.join(" | ");
}

namespace Models {
    FindAndReplaceModel::FindAndReplaceModel(QMLExtensions::ColorsModel *colorsModel, QObject *parent):
        QAbstractListModel(parent),
        Common::BaseEntity(),
        m_ColorsModel(colorsModel),
        m_Flags(Common::SearchFlags::None)
    {
        Q_ASSERT(colorsModel != nullptr);
        initDefaultFlags();
    }

    void FindAndReplaceModel::initArtworksList() {
        LOG_INFO << "Flags:" << searchFlagsToString(m_Flags);
        LOG_INFO << "ReplaceFrom: [" << m_ReplaceFrom << "]";

        Q_ASSERT(!m_ReplaceFrom.isEmpty());
        if (m_ReplaceFrom.isEmpty()) {
            LOG_WARNING << "Replace from is empty";
            return;
        }

        normalizeSearchCriteria();

        Models::FilteredArtItemsProxyModel *filteredItemsModel = m_CommandManager->getFilteredArtItemsModel();
        auto rawSnapshot = filteredItemsModel->getSearchablePreviewOriginalItems(m_ReplaceFrom, m_Flags);
        m_ArtworksSnapshot.set(rawSnapshot);

        LOG_INFO << "Found" << m_ArtworksSnapshot.size() << "item(s)";

        for (auto &locker: m_ArtworksSnapshot.getRawData()) {
            std::shared_ptr<PreviewArtworkElement> preview = std::dynamic_pointer_cast<PreviewArtworkElement>(locker);
            Q_ASSERT(preview);
            Models::ArtworkMetadata *artwork = locker->getArtworkMetadata();
            bool hasMatch = false;
            Common::SearchFlags flags = Common::SearchFlags::None;

            if (getSearchInTitle()) {
                flags = m_Flags;
                Common::UnsetFlag(flags, Common::SearchFlags::Description);
                Common::UnsetFlag(flags, Common::SearchFlags::Keywords);

                hasMatch = Helpers::hasSearchMatch(m_ReplaceFrom, artwork, flags);
                preview->setHasTitleMatch(hasMatch);
            }

            if (getSearchInDescription()) {
                flags = m_Flags;
                Common::UnsetFlag(flags, Common::SearchFlags::Title);
                Common::UnsetFlag(flags, Common::SearchFlags::Keywords);

                hasMatch = Helpers::hasSearchMatch(m_ReplaceFrom, artwork, flags);
                preview->setHasDescriptionMatch(hasMatch);
            }

            if (getSearchInKeywords()) {
                flags = m_Flags;
                Common::UnsetFlag(flags, Common::SearchFlags::Description);
                Common::UnsetFlag(flags, Common::SearchFlags::Title);

                hasMatch = Helpers::hasSearchMatch(m_ReplaceFrom, artwork, flags);
                preview->setHasKeywordsMatch(hasMatch);
            }
        }
    }

    int FindAndReplaceModel::rowCount(const QModelIndex &parent) const {
        Q_UNUSED(parent);
        return (int)m_ArtworksSnapshot.size();
    }

    QVariant FindAndReplaceModel::data(const QModelIndex &index, int role) const {
        int indexRow = index.row();
        if (indexRow < 0 || indexRow >= (int)m_ArtworksSnapshot.size()) {
            return QVariant();
        }

        const Models::PreviewArtworkElement *item = accessPreviewElement(indexRow);

        switch (role) {
        case PathRole:
            return item->getArtworkMetadata()->getThumbnailPath();
        case IsSelectedRole:
            return item->getIsSelected();
        case HasTitleMatchRole:
            return item->hasTitleMatch();
        case HasDescriptionMatchRole:
            return item->hasDescriptionMatch();
        case HasKeywordsMatchRole:
            return item->hasKeywordsMatch();
        case IsVideoRole: {
            Models::VideoArtwork *videoArtwork = dynamic_cast<Models::VideoArtwork*>(item->getArtworkMetadata());
            return videoArtwork != nullptr;
        }
        default:
            return QVariant();
        }
    }

    Qt::ItemFlags FindAndReplaceModel::flags(const QModelIndex &index) const {
        int row = index.row();
        if (row < 0 || row >= (int)m_ArtworksSnapshot.size()) {
            return Qt::ItemIsEnabled;
        }

        return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
    }

    bool FindAndReplaceModel::setData(const QModelIndex &index, const QVariant &value, int role) {
        int indexRow = index.row();
        if (indexRow < 0 || indexRow >= (int)m_ArtworksSnapshot.size()) {
            return false;
        }

        int roleToUpdate = 0;

        switch (role) {
            case EditIsSelectedRole:
                accessPreviewElement(indexRow)->setIsSelected(value.toBool());
                roleToUpdate = IsSelectedRole;
                break;
            default:
                return false;
        }

        emit dataChanged(index, index, QVector<int>() << roleToUpdate);
        return true;
    }

    QHash<int, QByteArray> FindAndReplaceModel::roleNames() const {
        QHash<int, QByteArray> roles;
        roles[PathRole] = "path";
        roles[IsSelectedRole] = "isselected";
        roles[EditIsSelectedRole] = "editisselected";
        roles[HasTitleMatchRole] = "hastitle";
        roles[HasDescriptionMatchRole] = "hasdescription";
        roles[HasKeywordsMatchRole] = "haskeywords";
        roles[IsVideoRole] = "isvideo";
        return roles;
    }

#if !defined(CORE_TESTS) && !defined(INTEGRATION_TESTS)
    void FindAndReplaceModel::initHighlighting(QQuickTextDocument *document) {
        Helpers::MetadataHighlighter *highlighter = new Helpers::MetadataHighlighter(m_ReplaceFrom,
                                                                                     this,
                                                                                     m_ColorsModel,
                                                                                     document->textDocument());

        Q_UNUSED(highlighter);
    }
#endif

    QString FindAndReplaceModel::getSearchTitle(int index) {
        QString text;

        if (index < 0 || index >= (int)m_ArtworksSnapshot.size()) {
            return text;
        }

        Models::PreviewArtworkElement *item = accessPreviewElement(index);
        Models::ArtworkMetadata *artwork = item->getArtworkMetadata();

        if (item->hasTitleMatch()) {
            text = filterText(artwork->getTitle());
        } else {
            auto title = artwork->getTitle();
            if (title.size() > PREVIEWOFFSET*2) {
                text = title.left(PREVIEWOFFSET*2) + " ...";
            } else {
                text = title;
            }
        }

        return text;
    }

    QString FindAndReplaceModel::getSearchDescription(int index) {
        QString text;

        if (index < 0 || index >= (int)m_ArtworksSnapshot.size()) {
            return text;
        }

        Models::PreviewArtworkElement *item = accessPreviewElement(index);
        Models::ArtworkMetadata *artwork = item->getArtworkMetadata();

        if (item->hasDescriptionMatch()) {
            text = filterText(artwork->getDescription());
        } else {
            auto description = artwork->getDescription();
            if (description.size() > PREVIEWOFFSET*2) {
                text = description.left(PREVIEWOFFSET*2) + " ...";
            } else {
                text = description;
            }
        }

        return text;
    }

    QString FindAndReplaceModel::getSearchKeywords(int index) {
        QString text;

        if (index < 0 || index >= (int)m_ArtworksSnapshot.size()) {
            return text;
        }

        Models::PreviewArtworkElement *item = accessPreviewElement(index);
        Models::ArtworkMetadata *artwork = item->getArtworkMetadata();
        QStringList list = artwork->getKeywords();

        if (item->hasKeywordsMatch()) {
            Qt::CaseSensitivity caseSensitivity = getCaseSensitive() ? Qt::CaseSensitive : Qt::CaseInsensitive;
            const bool wholeWords = getSearchWholeWords();

            QStringList listNew;
            for (auto &el: list) {
                if (!wholeWords) {
                    if (el.contains(m_ReplaceFrom, caseSensitivity)) {
                        listNew.append(el);
                    }
                } else if (QString::compare(m_ReplaceFrom, el, caseSensitivity) == 0) {
                    listNew.append(el);
                }
            }

            text.append(listNew.join(", "));
        } else {
            if (list.length() > PREVIEWKEYWORDSCOUNT) {
                QStringList part = list.mid(0, PREVIEWKEYWORDSCOUNT);
                text = part.join(", ") + " ...";
            } else {
                text = list.join(", ");
            }
        }

        return text;
    }

    void FindAndReplaceModel::replace() {
        LOG_INFO << "Flags:" << searchFlagsToString(m_Flags);

        std::shared_ptr<Commands::FindAndReplaceCommand> replaceCommand(new Commands::FindAndReplaceCommand(m_ArtworksSnapshot.getRawData(),
                                                                                                            m_ReplaceFrom,
                                                                                                            m_ReplaceTo,
                                                                                                            m_Flags));

        m_CommandManager->processCommand(replaceCommand);

        emit replaceSucceeded();
    }

    bool FindAndReplaceModel::anySearchDestination() const {
        return
                getSearchInDescription() ||
                getSearchInTitle() ||
                getSearchInKeywords();
    }

    void FindAndReplaceModel::resetModel() {
        LOG_DEBUG << "#";

        // let the model save last used flags
        // initDefaultFlags();
        clearArtworks();
        m_ReplaceFrom.clear();
        m_ReplaceTo.clear();
    }

    void FindAndReplaceModel::clearArtworks() {
        LOG_DEBUG << "#";
        beginResetModel();
        {
            m_ArtworksSnapshot.clear();
        }
        endResetModel();
    }

    QString FindAndReplaceModel::filterText(const QString &text) {
        int size = m_ReplaceFrom.size();

#ifndef QT_DEBUG
        if (text.size() <= 2*PREVIEWOFFSET) {
            return text;
        }
#endif

        QString result;
        Qt::CaseSensitivity caseSensitivity = getCaseSensitive() ? Qt::CaseSensitive : Qt::CaseInsensitive;
        int pos = 0;
        std::vector<int> hits;

        while (pos != -1) {
            pos = text.indexOf(m_ReplaceFrom, pos, caseSensitivity);
            if (pos >= 0) {
                hits.push_back(pos);
                pos += size;
            }
        }

        if (!hits.empty()) {
            result = Helpers::getUnitedHitsString(text, hits, PREVIEWOFFSET);
        }

        return result;
    }

    void FindAndReplaceModel::setAllSelected(bool isSelected) {
        LOG_INFO << "isSelected:" << isSelected;
        const size_t size = m_ArtworksSnapshot.size();

        for (size_t i = 0; i < size; i++) {
            accessPreviewElement(i)->setIsSelected(isSelected);
        }

        auto first = this->index(0);
        auto last = this->index(this->rowCount() - 1);
        emit dataChanged(first, last, QVector<int>() << IsSelectedRole);

        emit allSelectedChanged();
    }

    void FindAndReplaceModel::initDefaultFlags() {
        m_Flags = Common::SearchFlags::None;
        Common::SetFlag(m_Flags, Common::SearchFlags::CaseSensitive);
        Common::SetFlag(m_Flags, Common::SearchFlags::Title);
        Common::SetFlag(m_Flags, Common::SearchFlags::Description);
        Common::SetFlag(m_Flags, Common::SearchFlags::Keywords);
        Common::SetFlag(m_Flags, Common::SearchFlags::IncludeSpaces);
        // Common::SetFlag(m_Flags, Common::SearchFlags::ExactMatch);
    }

    void FindAndReplaceModel::normalizeSearchCriteria() {
        if (m_ReplaceFrom.trimmed().isEmpty()) {
            LOG_DEBUG << "Setting whole words search to false";
            setSearchWholeWords(false);
        }
    }

    PreviewArtworkElement *FindAndReplaceModel::accessPreviewElement(size_t index) const {
        Q_ASSERT(index < m_ArtworksSnapshot.size());
        auto &locker = m_ArtworksSnapshot.at(index);
        auto element = std::dynamic_pointer_cast<PreviewArtworkElement>(locker);
        Q_ASSERT(element);
        return element.get();
    }
}

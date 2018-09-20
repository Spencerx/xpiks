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
#include <Common/logging.h>
#include <Common/defines.h>
#include <Artworks/artworkmetadata.h>
#include <Artworks/videoartwork.h>
#include <Models/settingsmodel.h>
#include <Models/Editing/previewartworkelement.h>
#include <Helpers/filterhelpers.h>
#include <Helpers/metadatahighlighter.h>
#include <Helpers/cpphelpers.h>
#include <Helpers/stringhelper.h>
#include <Commands/Editing/findandreplacetemplate.h>
#include <Commands/Editing/modifyartworkscommand.h>
#include <Commands/Base/icommandmanager.h>

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
    FindAndReplaceModel::FindAndReplaceModel(QMLExtensions::ColorsModel &colorsModel,
                                             Commands::ICommandManager &commandManager,
                                             QObject *parent):
        QAbstractListModel(parent),
        m_ColorsModel(colorsModel),
        m_CommandManager(commandManager),
        m_Flags(Common::SearchFlags::None)
    {
        initDefaultFlags();
    }

    int FindAndReplaceModel::rowCount(const QModelIndex &parent) const {
        Q_UNUSED(parent);
        return (int)m_PreviewElements.size();
    }

    QVariant FindAndReplaceModel::data(const QModelIndex &index, int role) const {
        int indexRow = index.row();
        if (indexRow < 0 || indexRow >= (int)m_PreviewElements.size()) {
            return QVariant();
        }

        auto &element = m_PreviewElements[indexRow];

        switch (role) {
        case PathRole:
            return element->getArtwork()->getThumbnailPath();
        case IsSelectedRole:
            return element->getIsSelected();
        case HasTitleMatchRole:
            return element->hasTitleMatch();
        case HasDescriptionMatchRole:
            return element->hasDescriptionMatch();
        case HasKeywordsMatchRole:
            return element->hasKeywordsMatch();
        case IsVideoRole: {
            auto videoArtwork = std::dynamic_pointer_cast<Artworks::VideoArtwork>(element->getArtwork());
            return videoArtwork != nullptr;
        }
        default:
            return QVariant();
        }
    }

    Qt::ItemFlags FindAndReplaceModel::flags(const QModelIndex &index) const {
        int row = index.row();
        if (row < 0 || row >= (int)m_PreviewElements.size()) {
            return Qt::ItemIsEnabled;
        }

        return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
    }

    bool FindAndReplaceModel::setData(const QModelIndex &index, const QVariant &value, int role) {
        int indexRow = index.row();
        if (indexRow < 0 || indexRow >= (int)m_PreviewElements.size()) {
            return false;
        }

        int roleToUpdate = 0;

        switch (role) {
            case EditIsSelectedRole:
                m_PreviewElements[indexRow]->setIsSelected(value.toBool());
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

    void FindAndReplaceModel::updatePreviewFlags() {
        LOG_DEBUG << "#";
        for (auto &element: m_PreviewElements) {
            auto &artwork = element->getArtwork();
            bool hasMatch = false;
            Common::SearchFlags flags = Common::SearchFlags::None;

            if (getSearchInTitle()) {
                flags = m_Flags;
                Common::UnsetFlag(flags, Common::SearchFlags::Description);
                Common::UnsetFlag(flags, Common::SearchFlags::Keywords);

                hasMatch = Helpers::hasSearchMatch(m_ReplaceFrom, artwork, flags);
                element->setHasTitleMatch(hasMatch);
            }

            if (getSearchInDescription()) {
                flags = m_Flags;
                Common::UnsetFlag(flags, Common::SearchFlags::Title);
                Common::UnsetFlag(flags, Common::SearchFlags::Keywords);

                hasMatch = Helpers::hasSearchMatch(m_ReplaceFrom, artwork, flags);
                element->setHasDescriptionMatch(hasMatch);
            }

            if (getSearchInKeywords()) {
                flags = m_Flags;
                Common::UnsetFlag(flags, Common::SearchFlags::Description);
                Common::UnsetFlag(flags, Common::SearchFlags::Title);

                hasMatch = Helpers::hasSearchMatch(m_ReplaceFrom, artwork, flags);
                element->setHasKeywordsMatch(hasMatch);
            }
        }
    }

    void FindAndReplaceModel::findReplaceCandidates(const Artworks::ArtworksSnapshot &snapshot) {
        LOG_DEBUG << "size:" << snapshot.size();
        LOG_INFO << "flags:" << searchFlagsToString(m_Flags);
        LOG_INFO << "replace from: [" << m_ReplaceFrom << "]";

        Q_ASSERT(!m_ReplaceFrom.isEmpty());
        if (m_ReplaceFrom.isEmpty()) {
            LOG_WARNING << "Replace from is empty";
            return;
        }

        normalizeSearchCriteria();

        auto previewElements = Helpers::filterMap<std::shared_ptr<Artworks::ArtworkMetadata>,
                std::shared_ptr<PreviewArtworkElement>>(
                    snapshot.getRawData(),
                    [this](const std::shared_ptr<Artworks::ArtworkMetadata> &artwork) {
            return Helpers::hasSearchMatch(this->m_ReplaceFrom, artwork, this->m_Flags);
        },
        [](const std::shared_ptr<Artworks::ArtworkMetadata> &artwork) {
            return std::make_shared<PreviewArtworkElement>(artwork);
        });

        m_PreviewElements.swap(previewElements);
        LOG_INFO << "Found" << m_PreviewElements.size() << "item(s)";
    }

#if !defined(CORE_TESTS) && !defined(INTEGRATION_TESTS)
    void FindAndReplaceModel::initHighlighting(QQuickTextDocument *document) {
        Helpers::MetadataHighlighter *highlighter = new Helpers::MetadataHighlighter(m_ReplaceFrom,
                                                                                     this,
                                                                                     &m_ColorsModel,
                                                                                     document->textDocument());

        Q_UNUSED(highlighter);
    }
#endif

    QString FindAndReplaceModel::getSearchTitle(int index) {
        QString text;

        if (index < 0 || index >= (int)m_PreviewElements.size()) {
            return text;
        }

        auto &item = m_PreviewElements[index];
        auto &artwork = item->getArtwork();

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

        if (index < 0 || index >= (int)m_PreviewElements.size()) {
            return text;
        }

        auto &item = m_PreviewElements[index];
        auto &artwork = item->getArtwork();

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

        if (index < 0 || index >= (int)m_PreviewElements.size()) {
            return text;
        }

        auto &item = m_PreviewElements[index];
        auto &artwork = item->getArtwork();
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
        using namespace Commands;
        // this will clear m_ArtworksSnapshot since it will be
        // moved inside command but this behavior is desirable
        // in this place since it's the end of replacement flow
        using PreviewPtr = std::shared_ptr<PreviewArtworkElement>;
        using ArtworkPtr = std::shared_ptr<Artworks::ArtworkMetadata>;
        Artworks::ArtworksSnapshot snapshot(
                    Helpers::map<PreviewPtr, ArtworkPtr>(
                        m_PreviewElements,
                        [](PreviewPtr const &element) {
                        return element->getArtwork();
                    }));
        m_CommandManager.processCommand(
                    std::make_shared<ModifyArtworksCommand>(
                        std::move(snapshot),
                        std::make_shared<FindAndReplaceTemplate>(m_ReplaceFrom,
                                                                 m_ReplaceTo,
                                                                 m_Flags)));

        LOG_DEBUG << "Own snapshot is empty now";
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
            m_PreviewElements.clear();
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

        for (auto &element: m_PreviewElements) {
            element->setIsSelected(isSelected);
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
}

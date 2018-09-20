/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "warningsmodel.h"
#include <QObject>
#include <QStringList>
#include <Artworks/artworkmetadata.h>
#include <Common/flags.h>
#include <Artworks/imageartwork.h>
#include <Artworks/basickeywordsmodel.h>
#include <Helpers/indicesranges.h>
#include <Models/Artworks/artworkslistmodel.h>
#include "warningssettingsmodel.h"

namespace Warnings {
    void describeWarningFlags(Common::WarningFlags warningsFlags,
                              std::shared_ptr<Artworks::ArtworkMetadata> const &artwork,
                              const WarningsSettingsModel &settingsModel,
                              QStringList &descriptions) {

        if (Common::HasFlag(warningsFlags, Common::WarningFlags::SizeLessThanMinimum)) {
            auto image = std::dynamic_pointer_cast<Artworks::ImageArtwork>(artwork);
#ifdef QT_DEBUG
            Q_ASSERT(image != nullptr);
            {
#else
            if (image != NULL) {
#endif
                QSize size = image->getImageSize();
                int x = size.width();
                int y = size.height();
                descriptions.append(QObject::tr("Image size %1 x %2 is less than minimal").arg(x).arg(y));
            }
        }

        if (Common::HasFlag(warningsFlags, Common::WarningFlags::NoKeywords)) {
            descriptions.append(QObject::tr("Item has no keywords"));
        }

        if (Common::HasFlag(warningsFlags, Common::WarningFlags::TooFewKeywords)) {
            const int minKeywordsCount = settingsModel.getMinKeywordsCount();
            descriptions.append(QObject::tr("There's less than %1 keywords").arg(minKeywordsCount));
        }

        if (Common::HasFlag(warningsFlags, Common::WarningFlags::TooManyKeywords)) {
            Artworks::BasicKeywordsModel &keywordsModel = artwork->getBasicModel();
            descriptions.append(QObject::tr("There are too many keywords (%1)").arg(keywordsModel.getKeywordsCount()));
        }

        if (Common::HasFlag(warningsFlags, Common::WarningFlags::DescriptionIsEmpty)) {
            descriptions.append(QObject::tr("Description is empty"));
        }

        if (Common::HasFlag(warningsFlags, Common::WarningFlags::DescriptionNotEnoughWords)) {
            const int minWordsCount = settingsModel.getMinWordsCount();
            descriptions.append(QObject::tr("Description should not have less than %1 words").arg(minWordsCount));
        }

        if (Common::HasFlag(warningsFlags, Common::WarningFlags::DescriptionTooBig)) {
            descriptions.append(QObject::tr("Description is too long (%1 symbols)").arg(artwork->getDescription().length()));
        }

        if (Common::HasFlag(warningsFlags, Common::WarningFlags::TitleIsEmpty)) {
            descriptions.append(QObject::tr("Title is empty"));
        }

        if (Common::HasFlag(warningsFlags, Common::WarningFlags::TitleNotEnoughWords)) {
            const int minWordsCount = settingsModel.getMinWordsCount();
            descriptions.append(QObject::tr("Title should not have less than %1 words").arg(minWordsCount));
        }

        if (Common::HasFlag(warningsFlags, Common::WarningFlags::TitleTooManyWords)) {
            descriptions.append(QObject::tr("Title has too many words"));
        }

        if (Common::HasFlag(warningsFlags, Common::WarningFlags::TitleTooBig)) {
            descriptions.append(QObject::tr("Title is too long (%1 symbols)").arg(artwork->getTitle().length()));
        }

        if (Common::HasFlag(warningsFlags, Common::WarningFlags::SpellErrorsInKeywords)) {
            descriptions.append(QObject::tr("Keywords have spelling error(s)"));
        }

        if (Common::HasFlag(warningsFlags, Common::WarningFlags::SpellErrorsInDescription)) {
            descriptions.append(QObject::tr("Description has spelling error(s)"));
        }

        if (Common::HasFlag(warningsFlags, Common::WarningFlags::SpellErrorsInTitle)) {
            descriptions.append(QObject::tr("Title has spelling error(s)"));
        }

        if (Common::HasFlag(warningsFlags, Common::WarningFlags::ImageFileIsTooBig) ||
                Common::HasFlag(warningsFlags, Common::WarningFlags::VideoFileIsTooBig)) {
            double filesizeMB = 0.0;

            if (Common::HasFlag(warningsFlags, Common::WarningFlags::ImageFileIsTooBig)) {
                filesizeMB = settingsModel.getMaxImageFilesizeMB();
            } else if (Common::HasFlag(warningsFlags, Common::WarningFlags::VideoFileIsTooBig)) {
                filesizeMB = settingsModel.getMaxVideoFilesizeMB();
            }

            QString formattedSize = QString::number(filesizeMB, 'f', 1);
            descriptions.append(QObject::tr("File size is larger than %1 MB").arg(formattedSize));
        }

        if (Common::HasFlag(warningsFlags, Common::WarningFlags::KeywordsInDescription)) {
            descriptions.append(QObject::tr("Description contains some of the keywords"));
        }

        if (Common::HasFlag(warningsFlags, Common::WarningFlags::KeywordsInTitle)) {
            descriptions.append(QObject::tr("Title contains some of the keywords"));
        }

        if (Common::HasFlag(warningsFlags, Common::WarningFlags::FilenameSymbols)) {
            descriptions.append(QObject::tr("Filename contains special characters or spaces"));
        }

        if (Common::HasFlag(warningsFlags, Common::WarningFlags::VideoIsTooLong)) {
            QString formattedSeconds = QString::number(settingsModel.getMaxVideoDurationSeconds(), 'f', 1);
            descriptions.append(QObject::tr("Video is longer than %1 seconds").arg(formattedSeconds));
        }

        if (Common::HasFlag(warningsFlags, Common::WarningFlags::VideoIsTooShort)) {
            QString formattedSeconds = QString::number(settingsModel.getMinVideoDurationSeconds(), 'f', 1);
            descriptions.append(QObject::tr("Video is shorter than %1 seconds").arg(formattedSeconds));
        }
    }

    WarningsModel::WarningsModel(Models::ArtworksListModel &artworksListModel, WarningsSettingsModel &settings, QObject *parent):
        QSortFilterProxyModel(parent),
        m_WarningsSettingsModel(settings),
        m_ArtworksListModel(artworksListModel),
        m_ShowOnlySelected(false)
    {
        setSourceModel(&m_ArtworksListModel);
        QObject::connect(&m_WarningsSettingsModel, &WarningsSettingsModel::settingsUpdated,
                         this, &WarningsModel::warningsSettingsUpdated);

        QObject::connect(&artworksListModel, &QAbstractItemModel::rowsRemoved,
                         this, &WarningsModel::sourceRowsRemoved);
        QObject::connect(&artworksListModel, &QAbstractItemModel::rowsInserted,
                         this, &WarningsModel::sourceRowsInserted);
        QObject::connect(&artworksListModel, &QAbstractItemModel::modelReset,
                         this, &WarningsModel::sourceModelReset);
    }

    int WarningsModel::getMinKeywordsCount() const { return m_WarningsSettingsModel.getMinKeywordsCount(); }

    QStringList WarningsModel::describeWarnings(int index) const {
        QStringList descriptions;

        if (0 <= index && index < rowCount()) {
            int originalIndex = getOriginalIndex(index);
            std::shared_ptr<Artworks::ArtworkMetadata> artwork;
            if (m_ArtworksListModel.tryGetArtwork(originalIndex, artwork)) {
                Common::WarningFlags warningsFlags = artwork->getWarningsFlags();
                describeWarningFlags(warningsFlags, artwork, m_WarningsSettingsModel, descriptions);
            }
        }

        return descriptions;
    }

    void WarningsModel::update() {
        invalidateFilter();
        emit warningsCountChanged();
    }

    int WarningsModel::getOriginalIndex(int index) const {
        QModelIndex originalIndex = mapToSource(this->index(index, 0));
        int row = originalIndex.row();
        return row;
    }

    void WarningsModel::processPendingUpdates() {
        QAbstractItemModel *sourceItemModel = sourceModel();
        QVector<int> roles;
        roles << WarningsRole;

        Helpers::IndicesRanges ranges(m_PendingUpdates);

        for (auto &r: ranges.getRanges()) {
            QModelIndex indexFrom = mapFromSource(sourceItemModel->index(r.first, 0));
            QModelIndex indexTo = mapFromSource(sourceItemModel->index(r.second, 0));
            emit dataChanged(indexFrom, indexTo, roles);
        }

        m_PendingUpdates.clear();
    }

    void WarningsModel::onWarningsCouldHaveChanged(size_t originalIndex) {
        LOG_INFO << originalIndex;
        m_PendingUpdates.push_back((int)originalIndex);
    }

    void WarningsModel::onWarningsUpdateRequired() {
        update();
    }

    void WarningsModel::sourceRowsRemoved(QModelIndex, int, int) {
        emit warningsCountChanged();
    }

    void WarningsModel::sourceRowsInserted(QModelIndex, int, int) {
        emit warningsCountChanged();
    }

    void WarningsModel::sourceModelReset() {
        emit warningsCountChanged();
    }

    bool WarningsModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const {
        Q_UNUSED(sourceParent);
        bool rowIsOk = false;
        std::shared_ptr<Artworks::ArtworkMetadata> artwork;
        if (m_ArtworksListModel.tryGetArtwork(sourceRow, artwork) && !artwork->isRemoved()) {
            auto warningsFlags = artwork->getWarningsFlags();
            const bool anyWarnings = warningsFlags != Common::WarningFlags::None;

            rowIsOk = anyWarnings;

            if (m_ShowOnlySelected) {
                rowIsOk = artwork->isSelected() && anyWarnings;
            }
        }

        return rowIsOk;
    }

    QVariant WarningsModel::data(const QModelIndex &index, int role) const {
        if (role == WarningsRole) {
            return describeWarnings(index.row());
        } else {
            return QSortFilterProxyModel::data(index, role);
        }
    }

    QHash<int, QByteArray> WarningsModel::roleNames() const {
        QHash<int, QByteArray> roles = QSortFilterProxyModel::roleNames();
        roles[WarningsRole] = "warningsList";
        return roles;
    }
}

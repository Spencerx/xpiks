/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef DUPLICATESREVIEWMODEL_H
#define DUPLICATESREVIEWMODEL_H

#include <cstddef>
#include <memory>
#include <vector>

#include <QAbstractListModel>
#include <QHash>
#include <QObject>
#include <QString>
#include <QVariant>
#include <QVector>
#include <Qt>

#include "Artworks/artworkssnapshot.h"

class QByteArray;
class QModelIndex;

namespace Artworks {
    class BasicMetadataModel;
}

namespace QMLExtensions {
    class ColorsModel;
}

class QQuickTextDocument;

namespace SpellCheck {
    class IMetadataDuplicates;

    class DuplicatesReviewModel: public QAbstractListModel
    {
        Q_OBJECT
        using DuplicatesItem = std::shared_ptr<IMetadataDuplicates>;
    public:
        DuplicatesReviewModel(QMLExtensions::ColorsModel &colorsModel);

    public:
        enum DuplicatesReviewModel_Roles {
            PathRole = Qt::UserRole + 1,
            HasPathRole,
            OriginalIndexRole,
            IsVideoRole,
            HasVectorAttachedRole,
            BaseFilenameRole,
            TriggerRole
        };

    public:
        void setupModel(Artworks::BasicMetadataModel &basicModel);
        void setupModel(const Artworks::ArtworksSnapshot &snapshot);
        void clearDuplicates();

    public:
        Q_INVOKABLE void initTitleHighlighting(int index, QQuickTextDocument *document);
        Q_INVOKABLE void initDescriptionHighlighting(int index, QQuickTextDocument *document);
        Q_INVOKABLE void initKeywordsHighlighting(int index, QQuickTextDocument *document);
        Q_INVOKABLE QString getTitleDuplicates(int index);
        Q_INVOKABLE QString getDescriptionDuplicates(int index);
        Q_INVOKABLE QString getKeywordsDuplicates(int index);
        Q_INVOKABLE void clearModel();
        Q_INVOKABLE void processPendingUpdates();

        // QAbstractItemModel interface
    public:
        virtual int rowCount(const QModelIndex &parent) const override;
        virtual QVariant data(const QModelIndex &index, int role) const override;
        virtual QHash<int, QByteArray> roleNames() const override;

    signals:
        void rehighlightRequired();

    public slots:
        void onDuplicatesCouldHaveChanged(size_t originalIndex);

    private:
        std::vector<std::shared_ptr<IMetadataDuplicates>> m_DuplicatesList;
        QVector<size_t> m_PendingUpdates;
        QMLExtensions::ColorsModel &m_ColorsModel;
    };
}

#endif // DUPLICATESREVIEWMODEL_H

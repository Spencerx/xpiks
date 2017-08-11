/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2017 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef ABSTRACTLISTMODEL
#define ABSTRACTLISTMODEL

#include <QAbstractListModel>

#include <QVector>
#include <QList>
#include "../Helpers/indiceshelper.h"
#include "../Common/defines.h"

namespace Common {
    class AbstractListModel : public QAbstractListModel {
        Q_OBJECT
    public:
        AbstractListModel(QObject *parent = 0) : QAbstractListModel(parent) {}
        virtual ~AbstractListModel() {}

    public:
        virtual void removeItemsAtIndices(const QVector<QPair<int, int> > &ranges) {
            int rangesLength = Helpers::getRangesLength(ranges);
            LOG_INFO << "Ranges length:" << rangesLength;
            doRemoveItemsAtIndices(ranges, rangesLength);
        }

    protected:
        void updateItemsInRanges(const QVector<QPair<int, int> > &ranges, const QVector<int> &roles) {
            int rangesCount = ranges.count();
            for (int i = 0; i < rangesCount; ++i) {
                int startRow = ranges[i].first;
                int endRow = ranges[i].second;

                QModelIndex topLeft = index(startRow);
                QModelIndex bottomRight = index(endRow);
                emit dataChanged(topLeft, bottomRight, roles);
            }
        }

        virtual void removeInnerItem(int row) = 0;

        virtual bool shouldRemoveInRanges(int rangesLength) const { return rangesLength > 50; }

        virtual void removeInnerItemRange(int startRow, int endRow) {
            int count = endRow - startRow + 1;
            for (int j = 0; j < count; ++j) { removeInnerItem(startRow); }
        }

        void doRemoveItemsAtIndices(const QVector<QPair<int, int> > &ranges, int rangesLength) {
            int removedCount = 0;
            int rangesCount = ranges.count();

            bool willResetModel = shouldRemoveInRanges(rangesLength);

            if (willResetModel) {
                beginResetModel();
            }

            for (int i = 0; i < rangesCount; ++i) {
                int startRow = ranges[i].first - removedCount;
                int endRow = ranges[i].second - removedCount;

                if (!willResetModel) {
                    beginRemoveRows(QModelIndex(), startRow, endRow);
                }

                removeInnerItemRange(startRow, endRow);
                removedCount += (endRow - startRow + 1);

                if (!willResetModel) {
                    endRemoveRows();
                }
            }

            if (willResetModel) {
                endResetModel();
            }
        }
    };
}
#endif // ABSTRACTLISTMODEL


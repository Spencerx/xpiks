/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2015 Taras Kushnir <kushnirTV@gmail.com>
 *
 * Xpiks is distributed under the GNU General Public License, version 3.0
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SPELLCHECKERSERVICE_H
#define SPELLCHECKERSERVICE_H

#include <QObject>
#include <QString>
#include <QList>

namespace Models {
    class ArtworkMetadata;
}

namespace SpellCheck {
    class SpellCheckWorker;

    class SpellCheckerService : public QObject
    {
        Q_OBJECT
    public:
        SpellCheckerService();

    public:
        void startChecking();
        void submitItems(const QList<Models::ArtworkMetadata *> &artworksToCheck);
        void submitKeyword(Models::ArtworkMetadata* metadata, int keywordIndex);

    public:
        Q_INVOKABLE void cancelCurrentBatch();
        Q_INVOKABLE bool hasAnyPending();

    signals:
        void cancelSpellChecking();
        void spellCheckQueueIsEmpty();

    private slots:
        void workerQueueIsEmpty();

    private:
        SpellCheckWorker *m_SpellCheckWorker;
    };
}

#endif // SPELLCHECKERSERVICE_H

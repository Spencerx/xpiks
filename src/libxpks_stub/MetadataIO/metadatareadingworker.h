/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2017 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef METADATAREADINGWORKER_H
#define METADATAREADINGWORKER_H

#include <QByteArray>
#include <QObject>
#include <QProcess>
#include <QString>
#include <QStringList>

#include "Artworks/artworkssnapshot.h"

namespace Models {
    class SettingsModel;
}

namespace MetadataIO {
    class MetadataReadingHub;
}

namespace libxpks {
    namespace io {
        class ExiftoolImageReadingWorker : public QObject
        {
            Q_OBJECT
        public:
            explicit ExiftoolImageReadingWorker(Artworks::ArtworksSnapshot const &artworksToRead,
                                                Models::SettingsModel &settingsModel,
                                                MetadataIO::MetadataReadingHub &readingHub);
            virtual ~ExiftoolImageReadingWorker();

        signals:
            void stopped();

        public slots:
            void process();

        private slots:
            void innerProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

        public:
            void dismiss() { emit stopped(); }
            bool success() const { return m_ReadSuccess; }

        private:
            void initWorker();
            QStringList createArgumentsList();
            void parseExiftoolOutput(const QByteArray &output);
            void readSizes();

        private:
            Artworks::ArtworksSnapshot const &m_ItemsToReadSnapshot;
            MetadataIO::MetadataReadingHub &m_ReadingHub;
            QProcess *m_ExiftoolProcess;
            Models::SettingsModel &m_SettingsModel;
            volatile bool m_ReadSuccess;
        };
    }
}

#endif // METADATAREADINGWORKER_H

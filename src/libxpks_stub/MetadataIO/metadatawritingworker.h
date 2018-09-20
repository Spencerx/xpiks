/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2017 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef METADATAWRITINGWORKER_H
#define METADATAWRITINGWORKER_H

#include <QObject>
#include <QVector>
#include <QProcess>
#include <Artworks/artworkssnapshot.h>

namespace Helpers {
    class AsyncCoordinator;
}

namespace Models {
    class ArtworkMetadata;
    class SettingsModel;
}

namespace libxpks {
    namespace io {
        class ExiftoolImageWritingWorker : public QObject
        {
            Q_OBJECT
        public:
            explicit ExiftoolImageWritingWorker(Artworks::ArtworksSnapshot const &artworksToWrite,
                                                Helpers::AsyncCoordinator &asyncCoordinator,
                                                Models::SettingsModel &settingsModel,
                                                bool useBackups);
            virtual ~ExiftoolImageWritingWorker();

        public:
            bool success() const { return m_WriteSuccess; }

        signals:
            void stopped();

        public slots:
            void process();

        private slots:
            void innerProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

        private:
            void initWorker();
            QStringList createArgumentsList(const QString &jsonFilePath);
            void setArtworksSaved();

        private:
            QProcess *m_ExiftoolProcess;
            Artworks::ArtworksSnapshot m_ItemsToWriteSnapshot;
            Helpers::AsyncCoordinator &m_AsyncCoordinator;
            Models::SettingsModel &m_SettingsModel;
            bool m_UseBackups;
            bool m_WriteSuccess;
        };
    }
}

#endif // METADATAWRITINGWORKER_H

/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef ZIPARCHIVER_H
#define ZIPARCHIVER_H

#include <atomic>

#include <QFutureWatcher>
#include <QObject>
#include <QString>
#include <QStringList>

#include "Artworks/artworkssnapshot.h"
#include "Common/messages.h"
#include "Common/types.h"
#include "Models/iactionmodel.h"

template <class T1, class T2> class QHash;

namespace Models {
    using UnavailableFilesMessage = Common::NamedType<int, Common::MessageType::UnavailableFiles>;

    class ZipArchiver:
            public QObject,
            public Common::MessagesTarget<UnavailableFilesMessage>,
            public Models::IActionModel
    {
        Q_PROPERTY(int percent READ getPercent NOTIFY percentChanged)
        Q_PROPERTY(bool inProgress READ getInProgress NOTIFY inProgressChanged)
        Q_PROPERTY(bool isError READ getHasErrors NOTIFY hasErrorsChanged)
        Q_PROPERTY(int itemsCount READ getItemsCount NOTIFY itemsCountChanged)
        Q_OBJECT
    public:
        ZipArchiver();

    public:
        int getPercent() const;
        bool getInProgress() const { return m_IsInProgress; }
        bool getHasErrors() const { return m_HasErrors; }
        int getItemsCount() const { return static_cast<int>(m_ArtworksSnapshot.size()); }

    public:
        void setInProgress(bool value);
        void setHasErrors(bool value);

    public:
        virtual void handleMessage(UnavailableFilesMessage const &message) override;

        // IActionModel interface
    public:
        virtual std::shared_ptr<Commands::ICommand> getActionCommand(bool yesno) override;
        virtual void resetModel() override;

    public:
        void setArtworks(Artworks::ArtworksSnapshot const &snapshot);
        void resetArtworks();

    signals:
        void inProgressChanged();
        void hasErrorsChanged();
        void percentChanged();
        void itemsCountChanged();
        void startedProcessing();
        void finishedProcessing();
        void requestCloseWindow();

    public slots:
        void archiveCreated(int);
        void allFinished();

    protected:
        bool removeUnavailableItems();

    private:
        void fillFilenamesHash(QHash<QString, QStringList> &hash);
        void archiveArtworks();

#ifdef CORE_TESTS
    public:
#else
    protected:
#endif
        const Artworks::ArtworksSnapshot &getArtworksSnapshot() const { return m_ArtworksSnapshot; }

    private:
        Artworks::ArtworksSnapshot m_ArtworksSnapshot;
        QFutureWatcher<QStringList> m_ArchiveCreator;
        std::atomic_int m_ProcessedArtworksCount;
        volatile bool m_IsInProgress;
        volatile bool m_HasErrors;
    };
}

#endif // ZIPARCHIVER_H

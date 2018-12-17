/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "ziparchiver.h"

#include <cstddef>
#include <memory>

#include <QFileInfo>
#include <QHash>
#include <QList>
#include <QtConcurrentMap>
#include <QtDebug>

#include "Artworks/artworkmetadata.h"
#include "Artworks/artworkssnapshot.h"
#include "Artworks/imageartwork.h"
#include "Common/logging.h"
#include "Commands/Base/callbackcommand.h"
#include "Commands/Base/compositecommand.h"
#include "Commands/Editing/clearactionmodeltemplate.h"
#include "Helpers/cpphelpers.h"

#ifndef CORE_TESTS
#include "Helpers/ziphelper.h"
#endif

namespace Models {
    ZipArchiver::ZipArchiver():
        m_ArchiveCreator(this),
        m_IsInProgress(false),
        m_HasErrors(false)
    {
        QObject::connect(&m_ArchiveCreator, &QFutureWatcher<QStringList>::resultReadyAt,
                         this, &ZipArchiver::archiveCreated);
        QObject::connect(&m_ArchiveCreator, &QFutureWatcher<QStringList>::finished,
                         this, &ZipArchiver::allFinished);
    }

    int ZipArchiver::getPercent() const {
        const int artworksCount = getItemsCount();
        return artworksCount == 0 ? 0 : (m_ProcessedArtworksCount.load() * 100 / artworksCount);
    }

    void ZipArchiver::setInProgress(bool value) {
        if (m_IsInProgress != value) {
            m_IsInProgress = value;
            emit inProgressChanged();
        }
    }

    void ZipArchiver::setHasErrors(bool value) {
        if (m_HasErrors != value) {
            m_HasErrors = value;
            emit hasErrorsChanged();
        }
    }

    void ZipArchiver::handleMessage(UnavailableFilesMessage const &) {
        LOG_DEBUG << "#";
        removeUnavailableItems();
    }

    void ZipArchiver::archiveCreated(int) {
        m_ProcessedArtworksCount.fetch_add(1);
        emit percentChanged();
    }

    void ZipArchiver::allFinished() {
        LOG_DEBUG << "#";
        emit percentChanged();
        emit finishedProcessing();
        setInProgress(false);
    }

    std::shared_ptr<Commands::ICommand> ZipArchiver::getActionCommand(bool yesno) {
        if (yesno) {
            return std::make_shared<Commands::CompositeCommand>(
                        std::initializer_list<std::shared_ptr<Commands::ICommand>>{
                            std::make_shared<Commands::CallbackCommand>(
                            std::bind(&ZipArchiver::archiveArtworks, this))});
        } else {
            return std::make_shared<Commands::ClearActionModelCommand>(*this);
        }
    }

    void ZipArchiver::resetModel() {
        LOG_DEBUG << "#";
        resetArtworks();
        setHasErrors(false);
        setInProgress(false);
        m_ProcessedArtworksCount.store(0);
        emit percentChanged();
    }

    void ZipArchiver::setArtworks(Artworks::ArtworksSnapshot const &snapshot) {
        LOG_DEBUG << "#";
        auto imagesWithVectors = Helpers::filter<std::shared_ptr<Artworks::ArtworkMetadata>>(snapshot.getRawData(),
                                                                                             [](std::shared_ptr<Artworks::ArtworkMetadata> const &artwork) {
            auto image = std::dynamic_pointer_cast<Artworks::ImageArtwork>(artwork);
            return (image != nullptr) && image->hasVectorAttached();
        });
        m_ArtworksSnapshot.set(imagesWithVectors);
        emit itemsCountChanged();
    }

    void ZipArchiver::resetArtworks() {
        LOG_DEBUG << "#";
        m_ArtworksSnapshot.clear();
        emit itemsCountChanged();
    }

    bool ZipArchiver::removeUnavailableItems() {
        LOG_DEBUG << "#";

        auto &artworksListOld = getArtworksSnapshot();
        Artworks::ArtworksSnapshot::Container artworksListNew;

        const size_t size = artworksListOld.size();
        for (size_t i = 0; i < size; ++i) {
            auto &item = artworksListOld.at(i);

            if (!item->isUnavailable()) {
                artworksListNew.push_back(item);
            }
        }

        bool anyUnavailable = artworksListNew.size() != m_ArtworksSnapshot.size();
        if (anyUnavailable) {
            m_ArtworksSnapshot.set(artworksListNew);

            if (m_ArtworksSnapshot.empty()) {
                emit requestCloseWindow();
            }

            emit itemsCountChanged();
        }

        return anyUnavailable;
    }

    void ZipArchiver::fillFilenamesHash(QHash<QString, QStringList> &hash) {
        auto &snapshot = getArtworksSnapshot();
        LOG_DEBUG << "Processing" << snapshot.size() << "item(s)";

        for (auto &artwork: snapshot) {
            const QString &filepath = artwork->getFilepath();

            QFileInfo fi(filepath);
            QString basename = fi.baseName();

            auto image = std::dynamic_pointer_cast<Artworks::ImageArtwork>(artwork);
            if (image != nullptr) {
                if (image->hasVectorAttached()) {
                    LOG_VERBOSE << filepath << "is zipping candidate";
                    if (!hash.contains(basename)) {
                        hash.insert(basename, QStringList());
                    }

                    hash[basename].append(filepath);
                    hash[basename].append(image->getAttachedVectorPath());
                } else {
                    LOG_VERBOSE << filepath << "does not have vector attached";
                }
            } else {
                LOG_VERBOSE << filepath << "is not an image";
            }
        }
    }

    void ZipArchiver::archiveArtworks() {
        LOG_DEBUG << getItemsCount() << "item(s) pending";

        m_ProcessedArtworksCount.store(0);
        emit percentChanged();

        QHash<QString, QStringList> itemsWithSameName;
        fillFilenamesHash(itemsWithSameName);

        if (itemsWithSameName.empty()) {
            LOG_INFO << "No items to zip. Exiting...";
            setInProgress(false);
            emit finishedProcessing();
            return;
        }

        setInProgress(true);
        setHasErrors(false);
        emit startedProcessing();

#if !defined(CORE_TESTS)
        QList<QStringList> items = itemsWithSameName.values();
        LOG_INFO << "Creating zip archives for" << items.length() << "item(s)";
        m_ArchiveCreator.setFuture(QtConcurrent::mapped(items, Helpers::zipFiles));
#else
        LOG_DEBUG << "Skipping real zipping";
#endif
    }
}

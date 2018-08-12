/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2017 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "restoresessiontest.h"
#include "../../xpiks-qt/Models/settingsmodel.h"
#include "../../xpiks-qt/Models/sessionmanager.h"
#include "../../xpiks-qt/MetadataIO/metadataiocoordinator.h"
#include "../../xpiks-qt/MetadataIO/artworkssnapshot.h"
#include "../../xpiks-qt/Models/artitemsmodel.h"
#include "../../xpiks-qt/Models/artworksrepository.h"
#include "../../xpiks-qt/Models/imageartwork.h"
#include "signalwaiter.h"
#include "testshelpers.h"

QString RestoreSessionTest::testName() {
    return QLatin1String("RestoreSessionTest");
}

void RestoreSessionTest::setup() {
    Models::SettingsModel *settingsModel = m_TestsApp.getSettingsModel();

    m_TestsApp.getSettingsModel().setUseSpellCheck(false);
    m_TestsApp.getSettingsModel().setSaveSession(true);
    m_TestsApp.getSettingsModel().setAutoFindVectors(true);
}

int RestoreSessionTest::doTest() {
    Models::ArtItemsModel *artItemsModel = m_TestsApp.getArtItemsModel();
    Models::SessionManager *sessionManager = m_TestsApp.getSessionManager();
    //VERIFY(sessionManager->itemsCount() == 0, "Session is not cleared");
    Models::ArtworksRepository *artworksRepository = m_TestsApp.getArtworksRepository();

    QList<QUrl> files;
    files << setupFilePathForTest("images-for-tests/pixmap/img_0007.jpg")
          << setupFilePathForTest("images-for-tests/pixmap/seagull-for-clear.jpg")
          << setupFilePathForTest("images-for-tests/pixmap/seagull.jpg")
          << setupFilePathForTest("images-for-tests/vector/026.jpg")
          << setupFilePathForTest("images-for-tests/vector/027.jpg")
          << setupFilePathForTest("images-for-tests/mixed/0267.jpg");

    MetadataIO::MetadataIOCoordinator *ioCoordinator = m_TestsApp.getMetadataIOCoordinator();
    SignalWaiter waiter;
    QObject::connect(ioCoordinator, SIGNAL(metadataReadingFinished()), &waiter, SIGNAL(finished()));

    int addedCount = artItemsModel->addLocalArtworks(files);
    VERIFY(addedCount == files.length(), "Failed to add file");
    ioCoordinator->continueReading(true);

    VERIFY(waiter.wait(20), "Timeout exceeded for reading metadata.");

    VERIFY(!ioCoordinator->getHasErrors(), "Errors in IO Coordinator while reading");

    sleepWaitUntil(10, [&]() {
        return sessionManager->itemsCount() == files.length();
    });
    VERIFY(sessionManager->itemsCount() == files.length(), "Session does not contain all files");

    MetadataIO::ArtworksSnapshot oldArtworksSnapshot(m_TestsApp.getArtworkList());

    artworksRepository->resetEverything();
    artItemsModel->fakeDeleteAllItems();
    LOG_DEBUG << "About to restore...";

    int restoredCount = sessionManager->restoreSession(*artworksRepository);

    if (this->getIsInMemoryOnly()) {
        VERIFY(restoredCount == 0, "Session restore should not work for memory-only mode");
        return 0;
    }

    VERIFY(addedCount == restoredCount, "Failed to properly restore");
    ioCoordinator->continueReading(true);

    VERIFY(waiter.wait(20), "Timeout exceeded for reading session metadata.");

    VERIFY(!ioCoordinator->getHasErrors(), "Errors in IO Coordinator while reading");

    MetadataIO::ArtworksSnapshot newArtworksSnapshot(m_TestsApp.getArtworkList());
    auto &oldArtworksList = oldArtworksSnapshot.getRawData();
    auto &newArtworksList = newArtworksSnapshot.getRawData();

    VERIFY(oldArtworksList.size() == newArtworksList.size(), "Old and new snapshots have different number of items");
    LOG_INFO << "Comparing" << oldArtworksList.size() << "items";

    for (size_t i = 0; i < newArtworksList.size(); i++) {
        auto oldItem = oldArtworksList.at(i)->getArtworkMetadata();
        auto newItem = newArtworksList.at(i)->getArtworkMetadata();

        VERIFY(oldItem->getItemID() != newItem->getItemID(), "Comparing same IDs");
        VERIFY(oldItem->getFilepath() == newItem->getFilepath(), "Filepaths don't match");
        VERIFY(oldItem->getTitle() == newItem->getTitle(), "Titles don't match");
        VERIFY(oldItem->getDescription() == newItem->getDescription(), "Descriptions don't match");
        VERIFY(oldItem->getKeywords() == newItem->getKeywords(), "Keywords don't match");

        Artworks::ImageArtwork *oldImage = dynamic_cast<Artworks::ImageArtwork*>(oldItem);
        Artworks::ImageArtwork *newImage = dynamic_cast<Artworks::ImageArtwork*>(newItem);
        if (oldImage != nullptr && newImage != nullptr) {
            VERIFY(oldImage->hasVectorAttached() == newImage->hasVectorAttached(), "Vector attachment lost");
            VERIFY(oldImage->getAttachedVectorPath() == newImage->getAttachedVectorPath(), "Vector filepath lost");
            VERIFY(oldImage->getImageSize().width() == newImage->getImageSize().width(), "Image widths don't match");
            VERIFY(oldImage->getImageSize().height() == newImage->getImageSize().height(), "Image heights don't match");
        }
    }

    // -------------------------------------
    // TODO: move below code away (maybe to perf tests)

    /*
    sessionManager->clearSession();

    QUrl path = getFilePathForTest("images-for-tests/pixmap/img_0007.jpg");
    QString pathString = path.toLocalFile();
    std::deque<Artworks::ArtworkMetadata *> metadataVector(10000, new Artworks::ArtworkMetadata(pathString, 0, 0));
    MetadataIO::SessionSnapshot sessionSnapshot(metadataVector);
    auto &snapshot = sessionSnapshot.getSnapshot();

    sessionManager->saveToFile(snapshot);

    sessionManager->clearSession();
    VERIFY(sessionManager->filesCount() == 0, "Session is not cleared");

    QTime timer;

    timer.start();
    {
        sessionManager->readSessionFromFile();
    }
    int elapsed = timer.elapsed();

    VERIFY(elapsed < 1000, "Session parsing is slow");
    VERIFY(sessionManager->filesCount() == 10000, "Session initialization failed");

    //qDeleteAll(metadataVector);
    */

    return 0;
}

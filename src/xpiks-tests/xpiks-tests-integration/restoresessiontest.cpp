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
#include <QUrl>
#include <QList>
#include "signalwaiter.h"
#include "testshelpers.h"
#include "xpikstestsapp.h"

QString RestoreSessionTest::testName() {
    return QLatin1String("RestoreSessionTest");
}

void RestoreSessionTest::setup() {
    m_TestsApp.getSettingsModel().setUseSpellCheck(false);
    m_TestsApp.getSettingsModel().setAutoFindVectors(true);
    m_TestsApp.getSettingsModel().setSaveSession(true);
}

int RestoreSessionTest::doTest() {
    Models::SessionManager &sessionManager = m_TestsApp.getSessionManager();
    //VERIFY(sessionManager.itemsCount() == 0, "Session is not cleared");

    QList<QUrl> files;
    files << setupFilePathForTest("images-for-tests/pixmap/img_0007.jpg")
          << setupFilePathForTest("images-for-tests/pixmap/seagull-for-clear.jpg")
          << setupFilePathForTest("images-for-tests/pixmap/seagull.jpg")
          << setupFilePathForTest("images-for-tests/vector/026.jpg")
          << setupFilePathForTest("images-for-tests/vector/027.jpg")
          << setupFilePathForTest("images-for-tests/mixed/0267.jpg");

    VERIFY(m_TestsApp.addFilesForTest(files), "Failed to add files");

    sleepWaitUntil(10, [&]() {
        return sessionManager.itemsCount() == files.length();
    });
    VERIFY(sessionManager.itemsCount() == files.length(), "Session does not contain all files");

    Artworks::ArtworksSnapshot oldArtworksSnapshot = std::move(m_TestsApp.getArtworksListModel().createArtworksSnapshot());

    m_TestsApp.deleteAllArtworks();
    LOG_DEBUG << "About to restore...";

    SignalWaiter waiter;
    m_TestsApp.connectWaiterForImport(waiter);
    int restoredCount = m_TestsApp.restoreSavedSession();

    if (this->getIsInMemoryOnly()) {
        VERIFY(restoredCount == 0, "Session restore should not work for memory-only mode");
        return 0;
    }

    VERIFY(files.size() == restoredCount, "Failed to properly restore");
    VERIFY(m_TestsApp.continueReading(waiter), "Failed to reimport session");

    Artworks::ArtworksSnapshot newArtworksSnapshot = std::move(m_TestsApp.getArtworksListModel().createArtworksSnapshot());
    auto &oldArtworksList = oldArtworksSnapshot.getRawData();
    auto &newArtworksList = newArtworksSnapshot.getRawData();

    VERIFY(oldArtworksList.size() == newArtworksList.size(), "Old and new snapshots have different number of items");
    LOG_INFO << "Comparing" << oldArtworksList.size() << "items";

    for (size_t i = 0; i < newArtworksList.size(); i++) {
        auto oldItem = oldArtworksList.at(i);
        auto newItem = newArtworksList.at(i);

        VERIFY(oldItem->getItemID() != newItem->getItemID(), "Comparing same IDs");
        VERIFY(oldItem->getFilepath() == newItem->getFilepath(), "Filepaths don't match");
        VERIFY(oldItem->getTitle() == newItem->getTitle(), "Titles don't match");
        VERIFY(oldItem->getDescription() == newItem->getDescription(), "Descriptions don't match");
        VERIFY(oldItem->getKeywords() == newItem->getKeywords(), "Keywords don't match");

        auto oldImage = std::dynamic_pointer_cast<Artworks::ImageArtwork>(oldItem);
        auto newImage = std::dynamic_pointer_cast<Artworks::ImageArtwork>(newItem);
        VERIFY (oldImage != nullptr && newImage != nullptr, "Artworks are not images");
        VERIFY(oldImage->hasVectorAttached() == newImage->hasVectorAttached(), "Vector attachment lost");
        VERIFY(oldImage->getAttachedVectorPath() == newImage->getAttachedVectorPath(), "Vector filepath lost");
        VERIFY(oldImage->getImageSize().width() == newImage->getImageSize().width(), "Image widths don't match");
        VERIFY(oldImage->getImageSize().height() == newImage->getImageSize().height(), "Image heights don't match");
    }

    // -------------------------------------
    // TODO: move below code away (maybe to perf tests)

    /*
    sessionManager.clearSession();

    QUrl path = getFilePathForTest("images-for-tests/pixmap/img_0007.jpg");
    QString pathString = path.toLocalFile();
    std::deque<Artworks::ArtworkMetadata *> metadataVector(10000, new Artworks::ArtworkMetadata(pathString, 0, 0));
    MetadataIO::SessionSnapshot sessionSnapshot(metadataVector);
    auto &snapshot = sessionSnapshot.getSnapshot();

    sessionManager.saveToFile(snapshot);

    sessionManager.clearSession();
    VERIFY(sessionManager.filesCount() == 0, "Session is not cleared");

    QTime timer;

    timer.start();
    {
        sessionManager.readSessionFromFile();
    }
    int elapsed = timer.elapsed();

    VERIFY(elapsed < 1000, "Session parsing is slow");
    VERIFY(sessionManager.filesCount() == 10000, "Session initialization failed");

    //qDeleteAll(metadataVector);
    */

    return 0;
}

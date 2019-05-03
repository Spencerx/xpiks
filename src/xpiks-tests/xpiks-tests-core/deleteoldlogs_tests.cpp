#include "deleteoldlogs_tests.h"

#include <algorithm>

#include <QtGlobal>

#include "Services/Maintenance/filescleanup.h"

#define MEGABYTE_SIZE (1024*1024)

using CleanupCnd = Maintenance::FilesCleanup::CleanupCandidate;

QVector<CleanupCnd> createTooManyLogs(int logsCount) {
    QVector<CleanupCnd> logs;
    int N = logsCount;

    while (logsCount--) {
        logs.append({
            QString("xpiks-qt-01022015-%1-many.log").arg(N - 1 - logsCount), // m_FilePath
            1, // m_SizeBytes
            1 // m_AgeDays
        });
    }

    return logs;
}

QVector<CleanupCnd> createOldLogs(int logsCount, int startDay) {
    QVector<CleanupCnd> logs;
    int N = logsCount;

    while (logsCount--) {
        int index = N - 1 - logsCount;
        logs.append({
            QString("xpiks-qt-01022015-%1-old.log").arg(index), // m_FilePath
            1, // m_SizeBytes
            startDay + index // m_AgeDays
        });
    }

    return logs;
}

QVector<CleanupCnd> createBigLogs(int logsCount) {
    QVector<CleanupCnd> logs;
    int N = logsCount;

    while (logsCount--) {
        logs.append({
            QString("xpiks-qt-01022015-%1-big.log").arg(N - 1 - logsCount), // m_FilePath
            1 * MEGABYTE_SIZE, // m_SizeBytes
            0 // m_AgeDays
        });
    }

    return logs;
}

void DeleteOldLogsTest::deleteNoLogsTest() {
    auto logsToDelete = Maintenance::FilesCleanup(QVector<CleanupCnd>()).DryRun(100 /*maxFiles*/, 61 /*maxDays*/, 10*MEGABYTE_SIZE /*maxSize*/);
    QCOMPARE(logsToDelete.size(), 0);
}

void DeleteOldLogsTest::dontDeleteTest() {
    QVector<CleanupCnd> logsToDelete;

    logsToDelete = Maintenance::FilesCleanup(createOldLogs(60, 0))
                       .DryRun(100 /*maxFiles*/, 61 /*maxDays*/, MEGABYTE_SIZE /*maxSize*/);
    QCOMPARE(logsToDelete.size(), 0);

    logsToDelete = Maintenance::FilesCleanup(createBigLogs(9))
                       .DryRun(100 /*maxFiles*/, 60 /*maxDays*/, MEGABYTE_SIZE * 10 /*maxSize*/);
    QCOMPARE(logsToDelete.size(), 0);

    logsToDelete = Maintenance::FilesCleanup(createTooManyLogs(99))
                       .DryRun(100 /*maxFiles*/, 60 /*maxDays*/, MEGABYTE_SIZE /*maxSize*/);
    QCOMPARE(logsToDelete.size(), 0);
}

void DeleteOldLogsTest::deleteOldTest() {
    auto logsToDelete = Maintenance::FilesCleanup(createOldLogs(121, 0))
                            .DryRun(100 /*maxFiles*/, 60 /*maxDays*/, MEGABYTE_SIZE /*maxSize*/);
    QCOMPARE(logsToDelete.size(), 121 - 60);
}

void DeleteOldLogsTest::deleteLargeTest() {
    auto logsToDelete = Maintenance::FilesCleanup(createBigLogs(12))
                            .DryRun(100 /*maxFiles*/, 60 /*maxDays*/, 10*MEGABYTE_SIZE /*maxSize*/);
    QCOMPARE(logsToDelete.size(), 3);
}

void DeleteOldLogsTest::deleteManyTest() {
    auto logsToDelete = Maintenance::FilesCleanup(createTooManyLogs(113))
                            .DryRun(100 /*maxFiles*/, 60 /*maxDays*/, MEGABYTE_SIZE / 2 /*maxSize*/);
    QCOMPARE(logsToDelete.size(), 13);
}

void DeleteOldLogsTest::deleteCombinedTest() {
    QVector<CleanupCnd> logs;

    logs << createTooManyLogs(100) << createOldLogs(70, 60) << createBigLogs(12);
    auto logsToDelete = Maintenance::FilesCleanup(std::move(logs))
                            .DryRun(100 /*maxFiles*/, 59 /*maxDays*/, 10*MEGABYTE_SIZE /*maxSize*/);
    // logs removed until few big logs are removed
    QCOMPARE(logsToDelete.size(), (100 + 70 + 3));
}


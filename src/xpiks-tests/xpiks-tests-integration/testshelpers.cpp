#include "testshelpers.h"

#include <QChar>
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QList>
#include <QThread>
#include <QtGlobal>

#include "Helpers/filehelpers.h"

void ensureDirectoryExistsForFile(const QString &filepath) {
    QFileInfo fi(filepath);

    if (fi.exists()) { return; }

    QDir filesDir = fi.absoluteDir();
    if (!filesDir.exists()) {
        if (!filesDir.mkpath(".")) {
            qWarning() << "Failed to create a path:" << filesDir.path();
        }
    }
}

bool copyFile(const QString &from, const QString &to) {
    bool success = false;

    QFile destination(to);
    if (destination.exists()) {
        if (!destination.remove()) {
            return success;
        }
    }

    QFile source(from);
    if (source.exists()) {
        success = source.copy(to);
    }

    return success;
}

void sleepWaitUntil(int seconds, const std::function<bool ()> &condition) {
    int times = 0;
    bool becameTrue = false;

    while (times < seconds) {
        if (!condition()) {
            QCoreApplication::processEvents(QEventLoop::AllEvents);
            QThread::sleep(1);
            times++;
        } else {
            qDebug() << "Condition became true in" << (times + 1) << "try out of" << seconds;
            becameTrue = true;
            break;
        }
    }

    if (!becameTrue) {
        qDebug() << "Condition never was true";
    }
}

QString findFullPathForTests(const QString &prefix) {
    QString foundPath;
    if (!tryFindFullPathForTests(prefix, foundPath)) {
        foundPath = QFileInfo(prefix).absoluteFilePath();
    }
    return foundPath;
}

bool tryFindFullPathForTests(const QString &prefix, QString &path) {
    QFileInfo fi(prefix);
    int tries = 6;
    QStringList parents;
    while (tries--) {
        if (!fi.exists()) {
            parents.append("..");
            fi.setFile(parents.join('/') + "/" + prefix);
        } else {
            path = fi.absoluteFilePath();
            return true;
        }
    }

    return false;
}

QString findWildcartPathForTests(const QString &dirPath, const QStringList &filters) {
    Q_ASSERT(!filters.empty());
    QStringList parents = QStringList() << ".";
    int tries = 6;
    while (tries--) {
        QDir dir = QDir::current();
        dir.cd(parents.join("/") + "/" + dirPath);
        auto files = dir.entryInfoList(filters, QDir::Files);
        if (files.size() > 0) {
            return files.first().absoluteFilePath();
        } else {
            parents.append("..");
        }
    }

    return dirPath + "/" + filters.first();
}

QUrl setupFilePathForTest(const QString &root, const QString &prefix, bool withVector) {
    QString fullPath;
    if (tryFindFullPathForTests(prefix, fullPath)) {
        qInfo() << "Found artwork at" << fullPath;
        QString sessionPath = QDir::cleanPath(root + QChar('/') + prefix);
        ensureDirectoryExistsForFile(sessionPath);
        if (copyFile(fullPath, sessionPath)) {
            qInfo() << "Copied artwork to" << sessionPath;

            if (withVector) {
                QStringList vectors = Helpers::convertToVectorFilenames(fullPath);
                QStringList vectorPrefixes = Helpers::convertToVectorFilenames(prefix);
                Q_ASSERT(vectors.size() == vectorPrefixes.size());
                for (int i = 0; i < vectors.size(); i++) {
                    if (QFileInfo(vectors[i]).exists()) {
                        qInfo() << "Found vector at" << vectors[i];
                        QString vectorSessionPath = QDir::cleanPath(root + QChar('/') + vectorPrefixes[i]);
                        copyFile(vectors[i], vectorSessionPath);
                        qInfo() << "Copied vector to" << vectorSessionPath;
                        break;
                    }
                }
            }

            return QUrl::fromLocalFile(sessionPath);
        } else {
            return QUrl::fromLocalFile(fullPath);
        }
    }

    return QUrl::fromLocalFile(prefix);
}

#include "testshelpers.h"
#include <QThread>
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFileInfo>

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
    QFileInfo fi(prefix);
    int tries = 6;
    QStringList parents;
    while (tries--) {
        if (!fi.exists()) {
            parents.append("..");
            fi.setFile(parents.join('/') + "/" + prefix);
        } else {
            return fi.absoluteFilePath();
        }
    }

    return QFileInfo(prefix).absoluteFilePath();
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

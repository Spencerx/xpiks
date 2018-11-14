#ifndef SIGNALWAITER
#define SIGNALWAITER

#include <QDebug>
#include <QEventLoop>
#include <QObject>
#include <QSignalSpy>
#include <QString>
#include <QTimer>
#include <QtGlobal>

class SignalWaiter: public QObject {
    Q_OBJECT
public:
    SignalWaiter(QObject *parent=nullptr):
        QObject(parent),
        m_SignalSpy(this, SIGNAL(finished()))
    {
    }

    bool wait(int timeoutSeconds=5) {
        if (m_SignalSpy.count() > 0) {
            qDebug() << "Got the signal before waiting";
            m_SignalSpy.clear();
            return true;
        }

        QEventLoop loop;
        QObject::connect(this, SIGNAL(finished()), &loop, SLOT(quit()));

        QTimer timeoutTimer;
        timeoutTimer.setSingleShot(true);
        QObject::connect(&timeoutTimer, SIGNAL(timeout()), &loop, SLOT(quit()));

        timeoutTimer.start(timeoutSeconds*1000);
        loop.exec();

        m_SignalSpy.clear();
        // true - no timeout
        // false - timeout

        return timeoutTimer.isActive();
    }

signals:
    void finished();

private:
    QSignalSpy m_SignalSpy;
};

#endif // SIGNALWAITER


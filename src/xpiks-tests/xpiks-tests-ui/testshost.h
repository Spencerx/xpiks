#ifndef TESTSHOST_H
#define TESTSHOST_H

#include <QObject>
#include <QString>

class QQmlEngine;
class XpiksUITestsApp;

class TestsHost : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int smallSleepTime READ getSmallSleepTime CONSTANT)
    Q_PROPERTY(int normalSleepTime READ getNormalSleepTime CONSTANT)
    Q_PROPERTY(bool ftpServerEnabled READ getFtpServerEnabled CONSTANT)
    Q_PROPERTY(bool isReady READ getIsReady NOTIFY isReadyChanged)

public:
    static TestsHost& getInstance()
    {
        static TestsHost instance; // Guaranteed to be destroyed.
        // Instantiated on first use.
        return instance;
    }

public:
    int getSmallSleepTime() const;
    int getNormalSleepTime() const;
    bool getIsReady() const { return m_IsReady; }
    bool getFtpServerEnabled() const;
    void qmlEngineCallback(QQmlEngine *engine);

public:
    Q_INVOKABLE void setup(const QString &testname, bool realFiles=false);
    Q_INVOKABLE void cleanupTest();
    Q_INVOKABLE void cleanup();

signals:
    void isReadyChanged(bool value);

public slots:

public:
    void setApp(XpiksUITestsApp *app);

private:
    void processPendingEvents();

private:
    explicit TestsHost(QObject *parent = 0);

    TestsHost(TestsHost const&);
    void operator=(TestsHost const&);

private:
    XpiksUITestsApp *m_XpiksApp;
    bool m_IsReady;
};

#endif // TESTSHOST_H

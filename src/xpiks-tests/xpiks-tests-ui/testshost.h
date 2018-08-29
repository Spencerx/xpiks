#ifndef TESTSHOST_H
#define TESTSHOST_H

#include <QObject>
#include <functional>

class QQmlEngine;
class XpiksUITestsApp;

class TestsHost : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isReady READ getIsReady NOTIFY isReadyChanged)

public:
    static TestsHost& getInstance()
    {
        static TestsHost instance; // Guaranteed to be destroyed.
        // Instantiated on first use.
        return instance;
    }

public:
    bool getIsReady() const { return m_IsReady; }
    void qmlEngineCallback(QQmlEngine *engine);

public:
    Q_INVOKABLE void setup();
    Q_INVOKABLE void cleanup();
    Q_INVOKABLE QString bump() { return QLatin1String("testsHost"); }

public:
    void setApp(XpiksUITestsApp *app);

private:
    explicit TestsHost(QObject *parent = 0);

    TestsHost(TestsHost const&);
    void operator=(TestsHost const&);

signals:
    void isReadyChanged();

public slots:

private:
    XpiksUITestsApp *m_XpiksApp;
    bool m_IsReady;
};

#endif // TESTSHOST_H

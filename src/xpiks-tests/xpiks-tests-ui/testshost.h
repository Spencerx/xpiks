#ifndef TESTSHOST_H
#define TESTSHOST_H

#include <QObject>
#include <functional>

class QQmlEngine;
class XpiksUITestsApp;

class TestsHost : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString scoreme READ getName NOTIFY nameChanged)
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

public:
    QString getName() const { return QLatin1String("testsHost"); }
    void setApp(XpiksUITestsApp *app);

private:
    explicit TestsHost(QObject *parent = 0);

    TestsHost(TestsHost const&);
    void operator=(TestsHost const&);

signals:
    void isReadyChanged();
    void nameChanged();

public slots:

private:
    XpiksUITestsApp *m_XpiksApp;
    bool m_IsReady;
};

#endif // TESTSHOST_H

#include "testshost.h"

#include <QDebug>
#include <QEventLoop>
#include <QFont>
#include <QImage>
#include <QPainter>
#include <QPen>
#include <QQmlEngine>
#include <QQuickImageProvider>
#include <QSize>
#include <QStringList>
#include <Qt>
#include <QtGlobal>

#include "Common/logging.h"

#include "xpiksuitestsapp.h"

class FakeImageProvider: public QQuickImageProvider
{
public:
    FakeImageProvider()
        : QQuickImageProvider(QQuickImageProvider::Image)
    {
    }

public:
    virtual QImage requestImage(const QString &id, QSize *size, const QSize& requestedSize) override {
        Q_UNUSED(id);
        Q_UNUSED(requestedSize);

        const int width = 300;
        const int height = 300;

        if (size) {
            *size = QSize(width, height);
        }

        QImage img(width, height, QImage::Format_RGB888);
        img.fill(Qt::black);

        QPainter p;
        if (p.begin(&img)) {
            p.setPen(QPen(Qt::white));
            p.setFont(QFont("Times", 24, QFont::Bold));
            p.drawText(img.rect(), Qt::AlignCenter, id);
            p.end();
        }

        return img;
    }
};

TestsHost::TestsHost(QObject *parent) :
    QObject(parent),
    m_IsReady(false)
{
}

int TestsHost::getSmallSleepTime() const {
#if defined(TRAVIS_CI) || defined(APPVEYOR)
    return 500;
#else
    return 300;
#endif
}

int TestsHost::getNormalSleepTime() const {
#if defined(TRAVIS_CI) || defined(APPVEYOR)
    return 800;
#else
    return 600;
#endif
}

bool TestsHost::getFtpServerEnabled() const {
#if defined(WITH_FTP_SERVER)
    return true;
#else
    return false;
#endif
}

void TestsHost::qmlEngineCallback(QQmlEngine *engine) {
    LOG_DEBUG << "#";
    m_XpiksApp->setupUI(engine->rootContext());    

    engine->addImageProvider("cached", new FakeImageProvider());
    engine->addImageProvider("global", new FakeImageProvider());

    for (auto &path: m_XpiksApp->getQmlImportPaths()) {
        engine->addImportPath(path);
    }
}

void TestsHost::setup(const QString &testname, bool realFiles) {
    LOG_INFO << testname;
    m_XpiksApp->setupUITests(realFiles);
    m_IsReady = true;
    emit isReadyChanged(m_IsReady);
    processPendingEvents();
}

void TestsHost::cleanupTest() {
    LOG_DEBUG << "#";
    m_XpiksApp->cleanupTest();
    m_IsReady = false;
    emit isReadyChanged(m_IsReady);
    processPendingEvents();
}

void TestsHost::cleanup() {
    LOG_DEBUG << "#";
    m_XpiksApp->cleanup();
    processPendingEvents();
}

void TestsHost::setApp(XpiksUITestsApp *app) {
    Q_ASSERT(app != nullptr);
    m_XpiksApp = app;
}

void TestsHost::processPendingEvents() {
    QEventLoop loop;
    loop.processEvents(QEventLoop::AllEvents);
}

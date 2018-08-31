#include "testshost.h"
#include <QQmlEngine>
#include <QQmlContext>
#include "xpiksuitestsapp.h"
#include <Common/logging.h>

TestsHost::TestsHost(QObject *parent) :
    QObject(parent),
    m_IsReady(false)
{
}

void TestsHost::qmlEngineCallback(QQmlEngine *engine) {
    m_XpiksApp->setupUI(engine->rootContext());
}

void TestsHost::setup() {
    LOG_DEBUG << "#";
    m_XpiksApp->setupUITests();
    m_IsReady = true;
    emit isReadyChanged();
    processPendingEvents();
}

void TestsHost::cleanup() {
    LOG_DEBUG << "#";
    m_XpiksApp->cleanup();
    m_IsReady = false;
    emit isReadyChanged();
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

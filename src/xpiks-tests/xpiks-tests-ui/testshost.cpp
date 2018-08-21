#include "testshost.h"
#include <QQmlEngine>
#include <QQmlContext>
#include "xpiksuitestsapp.h"

TestsHost::TestsHost(QObject *parent) : QObject(parent)
{
}

void TestsHost::qmlEngineCallback(QQmlEngine *engine) {
    m_XpiksApp->setupUI(engine->rootContext());
}

void TestsHost::cleanup() {
    m_XpiksApp->cleanup();
}

void TestsHost::setApp(XpiksUITestsApp *app) {
    Q_ASSERT(app != nullptr);
    m_XpiksApp = app;
}

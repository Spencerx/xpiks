#include "testshost.h"
#include <QQmlEngine>
#include <QQmlContext>

TestsHost::TestsHost(QObject *parent) : QObject(parent)
{
    m_ColorsModel.initializeBuiltInThemes();
}

void TestsHost::qmlEngineCallback(QQmlEngine *engine) {
    engine->rootContext()->setContextProperty("uiColors", &m_ColorsModel);
}

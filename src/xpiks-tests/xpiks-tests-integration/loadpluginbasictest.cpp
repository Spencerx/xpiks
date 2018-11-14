#include "loadpluginbasictest.h"

#include <QLatin1String>
#include <QStringList>
#include <QUrl>

#include "Plugins/pluginmanager.h"

#include "testshelpers.h"
#include "xpikstestsapp.h"

QString LoadPluginBasicTest::testName() {
    return QLatin1String("LoadPluginBasicTest");
}

void LoadPluginBasicTest::setup() {
}

int LoadPluginBasicTest::doTest() {
#ifdef WIN32
    const QString dirPath = "plugins-for-tests/helloworld/xpiks-helloworld-plugin/debug";
#else
    const QString dirPath = "plugins-for-tests/helloworld/xpiks-helloworld-plugin";
#endif
    const QString p = "*xpiks-helloworld-plugin";
    QString pluginPath = findWildcartPathForTests(dirPath, QStringList() << (p + "*.dll") << (p + "*.so") << (p + "*.dylib"));

    Plugins::PluginManager &pluginManager = m_TestsApp.getPluginManager();
    bool success = pluginManager.installPlugin(QUrl::fromLocalFile(pluginPath));
    VERIFY(success, "Failed to install plugin");

    bool removed = pluginManager.removePlugin(0);
    VERIFY(removed, "Failed to remove plugin");

    return 0;
}

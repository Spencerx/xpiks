#ifndef XPIKSUITESTSAPP_H
#define XPIKSUITESTSAPP_H

#include <xpiksapp.h>

class XpiksUITestsApp: public XpiksApp
{
public:
    XpiksUITestsApp(Common::ISystemEnvironment &environment, QStringList const &importPaths = QStringList());

public:
    virtual void initialize() override;
    void waitInitialized();
    void cleanupTest();
    void cleanup();
    bool setupCommonFiles();
    void setupUITests();

public:
    QStringList const &getQmlImportPaths() const { return m_QmlImportPaths; }
    Models::ArtworkProxyModel &getArtworkProxyModel() { return m_ArtworkProxyModel; }

private:
    QStringList m_QmlImportPaths;
};

#endif // XPIKSUITESTSAPP_H

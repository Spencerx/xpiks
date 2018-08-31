#ifndef XPIKSUITESTSAPP_H
#define XPIKSUITESTSAPP_H

#include <xpiksapp.h>

class XpiksUITestsApp: public XpiksApp
{
public:
    XpiksUITestsApp(Common::ISystemEnvironment &environment);

public:
    virtual void initialize() override;
    void waitInitialized();
    void cleanup();
    bool setupCommonFiles();
    void setupUITests();

public:
    Models::ArtworkProxyModel &getArtworkProxyModel() { return m_ArtworkProxyModel; }
};

#endif // XPIKSUITESTSAPP_H

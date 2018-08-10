#ifndef XPIKSTESTSAPP_H
#define XPIKSTESTSAPP_H

#include <xpiksapp.h>

class XpiksTestsApp: public XpiksApp
{
public:
    XpiksTestsApp(Common::ISystemEnvironment &environment);

public:
    void waitInitialized();
    void cleanup();

public:
    bool addFilesForTest(QList<QUrl> const &urls);
    bool undoLastAction();
    void removeArtworks(Helpers::IndicesRanges const &ranges);

public:
    Artworks::ArtworkMetadata *getArtwork(int index);
    void setAutoFindVector(bool value);
    void setUseSpellCheck(bool value);
    void setUseAutoImport(bool value);

private:
    void doCleanup();

public:
    virtual void initialize() override;
};

#endif // XPIKSTESTSAPP_H

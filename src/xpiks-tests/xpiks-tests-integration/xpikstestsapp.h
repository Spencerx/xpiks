#ifndef XPIKSTESTSAPP_H
#define XPIKSTESTSAPP_H

#include <QVariant>
#include <xpiksapp.h>
#include <QMLExtensions/uicommandid.h>

class SignalWaiter;

class XpiksTestsApp: public XpiksApp
{
public:
    XpiksTestsApp(Common::ISystemEnvironment &environment);

public:
    void waitInitialized();
    void cleanup();

public:
    bool checkImportSucceeded(size_t importsCount=1);
    bool checkExportSucceeded();

public:
    void dispatch(QMLExtensions::UICommandID::CommandID id, QVariant const &value = QVariant());
    bool addFilesForTest(QList<QUrl> const &urls);
    void deleteArtworks(Helpers::IndicesRanges const &ranges);
    bool undoLastAction();

public:
    void connectWaiterForSpellcheck(SignalWaiter &waiter);
    void connectWaiterForImport(SignalWaiter &waiter);
    void connectWaiterForExport(SignalWaiter &waiter);

public:
    SpellCheck::UserDictionary &getUserDictionary() { return m_UserDictionary; }
    Models::ArtworksUploader &getArtworksUploader() { return m_ArtworksUploader; }
    Models::FilteredArtworksListModel &getFilteredArtworksModel() { return m_FilteredArtworksListModel; }
    Models::CombinedArtworksModel &getCombinedArtworksModel() { return m_CombinedArtworksModel; }
    Artworks::ArtworkMetadata *getArtwork(int index);

public:
    void setAutoFindVector(bool value);
    void setUseSpellCheck(bool value);
    void setUseAutoImport(bool value);

private:
    void doCleanup();

public:
    virtual void initialize() override;
};

#endif // XPIKSTESTSAPP_H

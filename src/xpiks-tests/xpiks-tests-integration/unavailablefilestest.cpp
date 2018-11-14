#include "unavailablefilestest.h"

#include <memory>

#include <QLatin1String>
#include <QList>
#include <QObject>
#include <QSignalSpy>
#include <QUrl>
#include <QVariant>

#include "Artworks/artworkmetadata.h"
#include "Models/Artworks/artworkslistmodel.h"
#include "Models/Artworks/artworksrepository.h"
#include "Models/Editing/artworkproxymodel.h"
#include "Models/settingsmodel.h"
#include "QMLExtensions/uicommandid.h"

#include "xpikstestsapp.h"

QString UnavailableFilesTest::testName() {
    return QLatin1String("UnavailableFilesTest");
}

void UnavailableFilesTest::setup() {
    m_TestsApp.getSettingsModel().setAutoFindVectors(false);
}

int UnavailableFilesTest::doTest() {
    QList<QUrl> files;
    files << setupFilePathForTest("images-for-tests/vector/026.jpg");

    VERIFY(m_TestsApp.addFilesForTest(files), "Failed to add files");

    QSignalSpy artworksListSpy(&m_TestsApp.getArtworksListModel(), SIGNAL(unavailableArtworksFound()));
    QSignalSpy proxySpy(&m_TestsApp.getArtworkProxyModel(), SIGNAL(itemBecomeUnavailable()));

    m_TestsApp.dispatch(QMLExtensions::UICommandID::SetupArtworkEdit, QVariant::fromValue(0));

    m_TestsApp.getArtworksRepository().insertIntoUnavailable(m_TestsApp.getArtwork(0)->getFilepath());
    m_TestsApp.getArtworksRepository().notifyUnavailableFiles();

    artworksListSpy.wait();
    VERIFY(artworksListSpy.count() == 1, "ArtworksList haven't discovered unavailable files");

    m_TestsApp.dispatch(QMLExtensions::UICommandID::RemoveUnavailableFiles, QVariant());

    proxySpy.wait();
    VERIFY(proxySpy.count() == 1, "ArtworkProxy didn't receive the signal");

    return 0;
}

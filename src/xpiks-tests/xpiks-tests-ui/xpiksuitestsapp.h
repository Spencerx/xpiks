#ifndef XPIKSUITESTSAPP_H
#define XPIKSUITESTSAPP_H

#include <QObject>
#include <QStringList>
#include <QUrl>

#include "xpiksapp.h"

namespace Common {
    class ISystemEnvironment;
}

namespace Models {
    class ArtworkProxyModel;
}

class XpiksUITestsApp: public XpiksApp
{
    Q_OBJECT
    Q_PROPERTY(QString sessionDirectory READ getSessionDirectory CONSTANT)
public:
    XpiksUITestsApp(Common::ISystemEnvironment &environment, QStringList const &importPaths = QStringList());

public:
    virtual void initialize() override;
    void waitInitialized();
    void cleanupTest();
    void cleanup();
    bool setupCommonFiles();
    void setupUITests(bool realFiles);
    QString getSessionDirectory() const;

public:
    Q_INVOKABLE bool uploadedFilesExist();
    Q_INVOKABLE bool csvExportIsValid(QString const &column0, QString const &column1);

private:
    void addFakeFiles();
    void addRealFiles();
    void doAddFiles(std::shared_ptr<Filesystem::IFilesCollection> const &files, int addCount);
    QUrl setupFilePathForTest(const QString &prefix, bool withVector=false);

public:
    QStringList const &getQmlImportPaths() const { return m_QmlImportPaths; }
    Models::ArtworkProxyModel &getArtworkProxyModel() { return m_ArtworkProxyModel; }

private:
    QStringList m_QmlImportPaths;
};

#endif // XPIKSUITESTSAPP_H

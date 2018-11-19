#ifndef XPIKSTESTSAPP_H
#define XPIKSTESTSAPP_H

#include <memory>

#include <QVariant>
#include <QList>

#include "Helpers/indicesranges.h"
#include "QMLExtensions/uicommandid.h"
#include "xpiksapp.h"

class QUrl;
namespace Artworks { class ArtworkMetadata; }
namespace AutoComplete { class AutoCompleteService; }
namespace AutoComplete { class KeywordsAutoCompleteModel; }
namespace Common { class ISystemEnvironment; }
namespace Encryption { class SecretsManager; }
namespace KeywordsPresets { class PresetKeywordsModel; }
namespace MetadataIO { class CsvExportModel; }
namespace MetadataIO { class MetadataIOCoordinator; }
namespace MetadataIO { class MetadataIOService; }
namespace Models { class ArtworkProxyModel; }
namespace Models { class ArtworksListModel; }
namespace Models { class ArtworksRepository; }
namespace Models { class ArtworksUploader; }
namespace Models { class CombinedArtworksModel; }
namespace Models { class FilteredArtworksListModel; }
namespace Models { class FindAndReplaceModel; }
namespace Models { class QuickBuffer; }
namespace Models { class SessionManager; }
namespace Models { class SettingsModel; }
namespace Models { class UploadInfoRepository; }
namespace Models { class ZipArchiver; }
namespace Plugins { class PluginManager; }
namespace SpellCheck { class SpellCheckService; }
namespace SpellCheck { class SpellCheckSuggestionModel; }
namespace SpellCheck { class UserDictEditModel; }
namespace SpellCheck { class UserDictionary; }
namespace Suggestion { class KeywordsSuggestor; }
namespace Translation { class TranslationManager; }

class SignalWaiter;

class XpiksTestsApp: public XpiksApp
{
public:
    XpiksTestsApp(Common::ISystemEnvironment &environment);

public:
    void waitInitialized();
    void cleanup();
    virtual void stop() override;

public:
    bool checkImportSucceeded(int importsCount=-1);
    bool checkExportSucceeded();

public:
    void dispatch(QMLExtensions::UICommandID::CommandID id, QVariant const &value = QVariant());
    bool addFilesForTest(QList<QUrl> const &urls);
    bool addDirectoriesForTest(QList<QUrl> const &urls);
    bool dropItemsForTest(QList<QUrl> const &urls);
    bool continueReading(SignalWaiter &waiter, bool ignoreBackups=true);
    void deleteArtworks(Helpers::IndicesRanges const &ranges);
    void deleteArtworksFromDirectory(int index);
    void deleteAllArtworks();
    bool undoLastAction();
    bool selectSpellSuggestions(int index);
    void selectAllArtworks();
    int restoreSavedSession();

public:
    void connectWaiterForSpellcheck(SignalWaiter &waiter);
    void connectWaiterForImport(SignalWaiter &waiter);
    void connectWaiterForExport(SignalWaiter &waiter);

public:
    SpellCheck::UserDictionary &getUserDictionary() { return m_UserDictionary; }
    Models::ArtworksUploader &getArtworksUploader() { return m_ArtworksUploader; }
    Models::ArtworksListModel &getArtworksListModel() { return m_ArtworksListModel; }
    Models::ArtworksRepository &getArtworksRepository() { return m_ArtworksRepository; }
    Models::FilteredArtworksListModel &getFilteredArtworksModel() { return m_FilteredArtworksListModel; }
    Models::CombinedArtworksModel &getCombinedArtworksModel() { return m_CombinedArtworksModel; }
    SpellCheck::SpellCheckSuggestionModel &getSpellSuggestionsModel() { return m_SpellSuggestionModel; }
    AutoComplete::AutoCompleteService &getAutoCompleteService() { return m_AutoCompleteService; }
    AutoComplete::KeywordsAutoCompleteModel &getKeywordsAutoCompleteModel() { return m_KeywordsAutoCompleteModel; }
    KeywordsPresets::PresetKeywordsModel &getKeywordsPresets() { return m_PresetsModel; }
    Models::SettingsModel &getSettingsModel() { return m_SettingsModel; }
    MetadataIO::CsvExportModel &getCsvExportModel() { return m_CsvExportModel; }
    Models::UploadInfoRepository &getUploadInfoRepository() { return m_UploadInfoRepository; }
    Models::FindAndReplaceModel &getFindAndReplaceModel() { return m_ReplaceModel; }
    MetadataIO::MetadataIOService &getMetadataIOService() { return m_MetadataIOService; }
    MetadataIO::MetadataIOCoordinator &getMetadataIOCoordinator() { return m_MetadataIOCoordinator; }
    Plugins::PluginManager &getPluginManager() { return m_PluginManager; }
    Suggestion::KeywordsSuggestor &getKeywordsSuggestor() { return m_KeywordsSuggestor; }
    Encryption::SecretsManager &getSecretsManager() { return m_SecretsManager; }
    Models::SessionManager &getSessionManager() { return m_SessionManager; }
    SpellCheck::SpellCheckService &getSpellCheckService() { return m_SpellCheckService; }
    Translation::TranslationManager &getTranslationManager() { return m_TranslationManager; }
    Models::QuickBuffer &getQuickBuffer() { return m_QuickBuffer; }
    SpellCheck::UserDictEditModel &getUserDictEditModel() { return m_UserDictEditModel; }
    Models::ZipArchiver &getZipArchiver() { return m_ZipArchiver; }
    Models::ArtworkProxyModel &getArtworkProxyModel() { return m_ArtworkProxyModel; }

public:
    std::shared_ptr<Artworks::ArtworkMetadata> getArtwork(int index);
    int getArtworksCount();

private:
    bool doContinueReading(SignalWaiter &waiter, bool ignoreBackups = true);

public:
    virtual void initialize() override;
};

#endif // XPIKSTESTSAPP_H

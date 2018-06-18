/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef MAINDELEGATOR_H
#define MAINDELEGATOR_H

#include <QObject>
#include <QStringList>
#include <QList>
#include <QUrl>
#include <QVector>
#include <QObject>
#include <memory>
#include <vector>
#include "../UndoRedo/ihistoryitem.h"
#include "../Connectivity/analyticsuserevent.h"
#include "../Common/flags.h"
#include "../Common/baseentity.h"
#include "icommandmanager.h"
#include "../Common/iservicebase.h"
#include "../Helpers/ifilenotavailablemodel.h"
#include "../Models/artworkelement.h"
#include "../KeywordsPresets/presetkeywordsmodel.h"
#include "../KeywordsPresets/presetkeywordsmodelconfig.h"
#include "../Helpers/asynccoordinator.h"
#include "../MetadataIO/artworkssnapshot.h"
#include "../Common/flags.h"

namespace Encryption {
    class SecretsManager;
}

namespace UndoRedo {
    class UndoRedoManager;
}

namespace Common {
    class BasicMetadataModel;
    class IMetadataOperator;
}

namespace Models {
    class ArtworksRepository;
    class ArtItemsModel;
    class FilteredArtworksListModel;
    class CombinedArtworksModel;
    class ArtworkUploader;
    class UploadInfoRepository;
    class UploadInfo;
    class ArtworkMetadata;
    class ZipArchiver;
    class SettingsModel;
    class RecentDirectoriesModel;
    class RecentFilesModel;
    class LogsModel;
    class LanguagesModel;
    class FindAndReplaceModel;
    class DeleteKeywordsViewModel;
    class UIManager;
    class ArtworkProxyBase;
    class ArtworkProxyModel;
    class SessionManager;
    class SwitcherModel;
}

namespace Suggestion {
    class KeywordsSuggestor;
}

namespace MetadataIO {
    class MetadataIOService;
    class MetadataIOCoordinator;
    class CsvExportModel;
}

namespace SpellCheck {
    class SpellCheckerService;
    class SpellCheckSuggestionModel;
    class DuplicatesReviewModel;
}

namespace Connectivity {
    class TelemetryService;
    class UpdateService;
    class RequestsService;
}

namespace Plugins {
    class PluginManager;
}

namespace Warnings {
    class WarningsService;
    class WarningsModel;
}

namespace QMLExtensions {
    class ColorsModel;
    class ImageCachingService;
    class VideoCachingService;
    class ArtworksUpdateHub;
}

namespace AutoComplete {
    class AutoCompleteService;
    class KeywordsAutoCompleteModel;
}

namespace Helpers {
    class HelpersQmlWrapper;
    class DatabaseManager;
}

namespace Translation {
    class TranslationService;
    class TranslationManager;
}

namespace QuickBuffer {
    class QuickBuffer;
}

namespace Maintenance {
    class MaintenanceService;
}

namespace Commands {
    class MainDelegator: public Common::BaseEntity
    {
    public:
        MainDelegator() {}

    public:
        virtual void addWarningsService(Common::IServiceBase<Common::IBasicArtwork, Common::WarningsCheckFlags> *service);
        virtual void addAvailabilityListener(Helpers::IFileNotAvailableModel *listener);

    public:
        void recordHistoryItem(std::unique_ptr<UndoRedo::IHistoryItem> &historyItem) const;
        void removeUnavailableFiles();

    public:
        void recodePasswords(const QString &oldMasterPassword,
                             const QString &newMasterPassword,
                             const std::vector<std::shared_ptr<Models::UploadInfo> > &uploadInfos) const;

        int reimportMetadata(const Artworks::ArtworksSnapshot &snapshot) const;
        void writeMetadata(const Artworks::WeakArtworksSnapshot &artworks, bool useBackups) const;
        void wipeAllMetadata(const Artworks::ArtworksSnapshot &artworks, bool useBackups) const;
        void updateArtworksAtIndices(const QVector<int> &indices) const;
        void updateArtworks(const Artworks::WeakArtworksSnapshot &artworks) const;
        void updateArtworks(const Artworks::ArtworksSnapshot::Container &artworks);
        void autoDiscoverExiftool() const;
        void cleanupOldXpksBackups(const QString &directory) const;

    public:
        void submitKeywordForSpellCheck(Common::BasicKeywordsModel *item, int keywordIndex) const;
        void submitForSpellCheck(const Artworks::WeakArtworksSnapshot &items) const;
        void submitForSpellCheck(const std::vector<Common::BasicKeywordsModel *> &items) const;
        void submitItemForSpellCheck(Common::BasicKeywordsModel *item, Common::SpellCheckFlags flags = Common::SpellCheckFlags::All) const;
        void checkSemanticDuplicates(Common::BasicKeywordsModel *item) const;
        void setupSpellCheckSuggestions(Common::IMetadataOperator *item, int index, Common::SuggestionFlags flags) const;
        void setupSpellCheckSuggestions(std::vector<std::pair<Common::IMetadataOperator *, int> > &itemPairs, Common::SuggestionFlags flags) const;
        void submitForSpellCheck(const std::vector<Common::BasicKeywordsModel *> &items, const QStringList &wordsToCheck) const;
        void setupDuplicatesModel(Common::BasicMetadataModel *item);
        void setupDuplicatesModel(const std::vector<Artworks::ArtworkMetadata *> &items) const;

    public:
        void submitKeywordsForWarningsCheck(Artworks::ArtworkMetadata *item) const;
        void submitForWarningsCheck(Artworks::ArtworkMetadata *item, Common::WarningsCheckFlags flags = Common::WarningsCheckFlags::All) const;
        void submitForWarningsCheck(const Artworks::WeakArtworksSnapshot &items) const;

    private:
        void submitForWarningsCheck(const std::vector<Common::IBasicArtwork *> &items) const;

    public:
        void saveArtworkBackup(Artworks::ArtworkMetadata *metadata) const;
        void saveArtworksBackups(const Artworks::WeakArtworksSnapshot &artworks) const;
        void reportUserAction(Connectivity::UserAction userAction) const;

        int restoreFiles(const QStringList &filenames, const QStringList &vectors);

    public:
        void restartSpellChecking();
        void disableSpellChecking();
        void disableDuplicatesCheck();
        void generateCompletions(const QString &prefix, Common::BasicKeywordsModel *source) const;

    public:
        void registerCurrentItem(Artworks::ArtworkMetadata *artwork) const;
        void registerCurrentItem(Models::ArtworkProxyBase *artworkProxy) const;
        void clearCurrentItem() const;

    private:
        Common::WordAnalysisFlags getWordAnalysisFlags() const;

    private:
        QVector<Common::IServiceBase<Common::IBasicArtwork, Common::WarningsCheckFlags> *> m_WarningsCheckers;
        QVector<Helpers::IFileNotAvailableModel*> m_AvailabilityListeners;
    };
}

#endif // MAINDELEGATOR_H

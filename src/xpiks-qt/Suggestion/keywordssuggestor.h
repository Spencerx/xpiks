/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef KEYWORDSSUGGESTOR_H
#define KEYWORDSSUGGESTOR_H

#include <atomic>
#include <memory>
#include <vector>

#include <QAbstractListModel>
#include <QHash>
#include <QModelIndex>
#include <QObject>
#include <QSet>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QVariant>
#include <Qt>

#include "Artworks/basickeywordsmodel.h"
#include "Common/messages.h"
#include "Common/statefulentity.h"
#include "Common/types.h"
#include "Connectivity/analyticsuserevent.h"
#include "Suggestion/suggestedkeywords.h"

class QByteArray;
class QModelIndex;

namespace Common {
    class ISystemEnvironment;
}

namespace Connectivity {
    class RequestsService;
}

namespace Models {
    class SettingsModel;
    class SwitcherModel;
    struct QuickBufferMessage;
}

namespace MetadataIO {
    class MetadataIOService;
}

namespace Microstocks {
    class IMicrostockAPIClients;
}

namespace Suggestion {
    class ISuggestionEngine;
    class SuggestionArtwork;

    class KeywordsSuggestor:
            public QAbstractListModel,
            public Common::MessagesSource<Common::NamedType<Connectivity::UserAction>>,
            public Common::MessagesSource<Models::QuickBufferMessage>
    {
        Q_OBJECT
        Q_PROPERTY(int suggestedKeywordsCount READ getSuggestedKeywordsCount NOTIFY suggestedKeywordsCountChanged)
        Q_PROPERTY(int otherKeywordsCount READ getOtherKeywordsCount NOTIFY otherKeywordsCountChanged)
        Q_PROPERTY(bool isInProgress READ getIsInProgress NOTIFY isInProgressChanged)
        Q_PROPERTY(int selectedArtworksCount READ getSelectedArtworksCount NOTIFY selectedArtworksCountChanged)
        Q_PROPERTY(int selectedSourceIndex READ getSelectedSourceIndex WRITE setSelectedSourceIndex NOTIFY selectedSourceIndexChanged)
        Q_PROPERTY(QString lastErrorString READ getLastErrorString WRITE setLastErrorString NOTIFY lastErrorStringChanged)
        Q_PROPERTY(bool isLocalSearch READ getIsLocalSearch NOTIFY isLocalSearchChanged)
        Q_PROPERTY(int searchTypeIndex READ getSearchTypeIndex WRITE setSearchTypeIndex NOTIFY searchTypeIndexChanged)
        Q_PROPERTY(QStringList engineNames READ getEngineNames NOTIFY engineNamesChanged)

    private:
        using Common::MessagesSource<Common::NamedType<Connectivity::UserAction>>::sendMessage;
        using Common::MessagesSource<Models::QuickBufferMessage>::sendMessage;

    public:
        KeywordsSuggestor(Models::SwitcherModel &switcherModel,
                          Models::SettingsModel &settingsModel,
                          Common::ISystemEnvironment &environment,
                          QObject *parent=nullptr);

    public:
        void setExistingKeywords(const QSet<QString> &keywords);
        void initSuggestionEngines(Microstocks::IMicrostockAPIClients &microstockClients,
                                   Connectivity::RequestsService &requestsService,
                                   MetadataIO::MetadataIOService &metadataIOService);
        void setSuggestedArtworks(std::vector<std::shared_ptr<SuggestionArtwork> > &suggestedArtworks);
        void clear();

        int getSelectedSourceIndex() const { return m_SelectedSourceIndex; }
        void setSelectedSourceIndex(int value);

        int getSearchTypeIndex() const;
        void setSearchTypeIndex(int value);

        void setLastErrorString(const QString &value) {
            if (value != m_LastErrorString) {
                m_LastErrorString = value;
                emit lastErrorStringChanged();
            }
        }

        void setSelectedArtworksCount(int value);

    public:
        int getSuggestedKeywordsCount() { return m_SuggestedKeywordsModel.rowCount(); }
        int getOtherKeywordsCount() { return m_OtherKeywordsModel.rowCount(); }
        bool getIsInProgress() const { return m_IsInProgress; }
        int getSelectedArtworksCount() const { return m_SelectedArtworksCount; }
        const QString &getLastErrorString() const { return m_LastErrorString; }
        bool getIsLocalSearch() const;
#ifdef INTEGRATION_TESTS
        size_t getEnginesCount() const { return m_QueryEngines.size(); }
#endif

    signals:
        void suggestedKeywordsCountChanged();
        void otherKeywordsCountChanged();
        void isInProgressChanged();
        void selectedSourceIndexChanged();
        void suggestionArrived();
        void selectedArtworksCountChanged();
        void lastErrorStringChanged();
        void isLocalSearchChanged();
        void searchTypeIndexChanged();
        void engineNamesChanged();

    private slots:
        void resultsAvailableHandler();
        void errorsReceivedHandler(const QString &error);
        void onProgressiveLoadTimer();
        void onSwitchesUpdated();

    public slots:
        void onLanguageChanged();

    private:
        void setInProgress() { m_IsInProgress = true; emit isInProgressChanged(); }
        void unsetInProgress() { m_IsInProgress = false; emit isInProgressChanged(); }
        bool isLocalSuggestionActive() const;

    public:
        Q_INVOKABLE void appendKeywordToSuggested(const QString &keyword);
        Q_INVOKABLE void appendKeywordToOther(const QString &keyword);
        Q_INVOKABLE QString removeSuggestedKeywordAt(int keywordIndex);
        Q_INVOKABLE QString removeOtherKeywordAt(int keywordIndex);
        Q_INVOKABLE void setArtworkSelected(int index, bool newState);
        Q_INVOKABLE void copyToQuickBuffer(int index) const;
        Q_INVOKABLE void searchArtworks(const QString &searchTerm, int resultsType);
        Q_INVOKABLE void cancelSearch();
        Q_INVOKABLE void close() { clear(); }
        Q_INVOKABLE QStringList getSuggestedKeywords() { return m_SuggestedKeywordsModel.getKeywords(); }
        /*Q_INVOKABLE*/ QStringList getEngineNames() const;
        Q_INVOKABLE QString getSuggestedKeywordsString() { return m_SuggestedKeywordsModel.getKeywordsString(); }
        Q_INVOKABLE void clearSuggested();
        Q_INVOKABLE void resetSelection();
        Q_INVOKABLE QObject *getSuggestedKeywordsModel();
        Q_INVOKABLE QObject *getAllOtherKeywordsModel();

    public:
        enum KeywordsSuggestor_Roles {
            UrlRole = Qt::UserRole + 1,
            IsSelectedRole,
            ExternalUrlRole
        };

        virtual int rowCount(const QModelIndex & parent = QModelIndex()) const override;
        virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const override;

    protected:
        virtual QHash<int, QByteArray> roleNames() const override;

    private:
        QSet<QString> getSelectedArtworksKeywords() const;
        void updateSuggestedKeywords();
        std::shared_ptr<ISuggestionEngine> getSelectedEngine();

#ifdef CORE_TESTS
    public:
        void setFakeSuggestions(std::vector<std::shared_ptr<SuggestionArtwork>> const &suggestions) {
            m_Suggestions = suggestions;
        }
        void initialize() { m_State.init(); }
#endif

    private:
        Common::StatefulEntity m_State;
        Models::SwitcherModel &m_SwitcherModel;
        Models::SettingsModel &m_SettingsModel;
        std::vector<std::shared_ptr<ISuggestionEngine>> m_QueryEngines;
        std::vector<std::shared_ptr<SuggestionArtwork>> m_Suggestions;
        SuggestedKeywords m_SuggestedKeywords;
        QString m_LastErrorString;
        Artworks::BasicKeywordsModel m_SuggestedKeywordsModel;
        Artworks::BasicKeywordsModel m_OtherKeywordsModel;
        // hack to load previews gradually
        QTimer m_ProgressiveLoadTimer;
        volatile int m_LoadedPreviewsNumber;
        int m_SelectedArtworksCount;
        int m_SelectedSourceIndex;
        int m_LocalSearchIndex;
        std::atomic_bool m_IsInProgress;
    };
}

#endif // KEYWORDSSUGGESTOR_H

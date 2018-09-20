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

#include <QAbstractListModel>
#include <QObject>
#include <QList>
#include <QHash>
#include <QSet>
#include <QTimer>
#include "suggestionartwork.h"
#include "isuggestionengine.h"
#include <Common/types.h>
#include <Common/messages.h>
#include <Common/statefulentity.h>
#include <Common/isystemenvironment.h>
#include <Artworks/basickeywordsmodel.h>
#include <Connectivity/requestsservice.h>
#include <Connectivity/analyticsuserevent.h>
#include <Models/Editing/quickbuffermessage.h>

namespace Models {
    class SwitcherModel;
    class SettingsModel;
}

namespace MetadataIO {
    class MetadataIOService;
}

namespace Microstocks {
    class IMicrostockAPIClients;
}

namespace Suggestion {
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
                          QObject *parent=NULL);

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

    public:
        int getSuggestedKeywordsCount() { return m_SuggestedKeywords.rowCount(); }
        int getOtherKeywordsCount() { return m_AllOtherKeywords.rowCount(); }
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
        void onLinearTimer();
        void onSwitchesUpdated();

    public slots:
        void onLanguageChanged();

    private:
        void setInProgress() { m_IsInProgress = true; emit isInProgressChanged(); }
        void unsetInProgress() { m_IsInProgress = false; emit isInProgressChanged(); }
        bool isLocalSuggestionActive() const;

    public:
        Q_INVOKABLE void appendKeywordToSuggested(const QString &keyword) { m_SuggestedKeywords.appendKeyword(keyword); emit suggestedKeywordsCountChanged(); }
        Q_INVOKABLE void appendKeywordToOther(const QString &keyword) { m_AllOtherKeywords.appendKeyword(keyword); emit otherKeywordsCountChanged(); }
        Q_INVOKABLE QString removeSuggestedKeywordAt(int keywordIndex);
        Q_INVOKABLE QString removeOtherKeywordAt(int keywordIndex);
        Q_INVOKABLE void setArtworkSelected(int index, bool newState);
        Q_INVOKABLE void copyToQuickBuffer(int index) const;
        Q_INVOKABLE void searchArtworks(const QString &searchTerm, int resultsType);
        Q_INVOKABLE void cancelSearch();
        Q_INVOKABLE void close() { clear(); }
        Q_INVOKABLE QStringList getSuggestedKeywords() { return m_SuggestedKeywords.getKeywords(); }
        /*Q_INVOKABLE*/ QStringList getEngineNames() const;
        Q_INVOKABLE QString getSuggestedKeywordsString() { return m_SuggestedKeywords.getKeywordsString(); }
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
        void accountKeywords(const QSet<QString> &keywords, int sign);
        QSet<QString> getSelectedArtworksKeywords() const;
        void updateSuggestedKeywords();
        void calculateBounds(int &lowerBound, int &upperBound) const;
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
        QString m_LastErrorString;
        QHash<QString, int> m_KeywordsHash;
        QSet<QString> m_ExistingKeywords;
        Artworks::BasicKeywordsModel m_SuggestedKeywords;
        Artworks::BasicKeywordsModel m_AllOtherKeywords;
        // hack to load previews gradually
        QTimer m_LinearTimer;
        volatile int m_LoadedPreviewsNumber;
        int m_SelectedArtworksCount;
        int m_SelectedSourceIndex;
        int m_LocalSearchIndex;
        volatile bool m_IsInProgress;
    };
}

#endif // KEYWORDSSUGGESTOR_H

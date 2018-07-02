/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SPELLCHECKSUGGESTIONMODEL_H
#define SPELLCHECKSUGGESTIONMODEL_H

#include <QAbstractListModel>
#include <QList>
#include <vector>
#include <memory>
#include <utility>
#include <Common/flags.h>
#include <Artworks/imetadataoperator.h>

namespace Models {
    class ArtworkMetadata;
}

namespace Common {
    class IMetadataOperator;
}

namespace Commands {
    class AppMessages;
}

namespace Artworks {
    class ArtworksSnapshot;
}

namespace SpellCheck {
    class SpellSuggestionsItem;
    class SpellCheckerService;
    class KeywordSpellSuggestions;

    typedef std::vector<std::shared_ptr<SpellSuggestionsItem> > SuggestionsVector;
    typedef std::vector<std::shared_ptr<KeywordSpellSuggestions> > KeywordsSuggestionsVector;

    class SpellCheckSuggestionModel : public QAbstractListModel
    {
        Q_OBJECT
        Q_PROPERTY(int artworksCount READ getArtworksCount NOTIFY artworksCountChanged)
        Q_PROPERTY(bool anythingSelected READ getAnythingSelected NOTIFY anythingSelectedChanged)

    public:
        SpellCheckSuggestionModel(SpellCheckerService &spellCheckerService, Commands::AppMessages &messages);
        virtual ~SpellCheckSuggestionModel();

    public:
        enum KeywordSpellSuggestions_Roles {
            WordRole = Qt::UserRole + 1,
            ReplacementIndexRole,
            ReplacementOriginRole
        };

    public:
        int getArtworksCount() const { return (int)m_CheckedItems.size(); }
        bool getAnythingSelected() const;

    public:
        Q_INVOKABLE QObject *getSuggestionItself(int index) const;
        Q_INVOKABLE void clearModel();
        Q_INVOKABLE void submitCorrections() const;
        Q_INVOKABLE void resetAllSuggestions();
        Q_INVOKABLE void updateSelection() { emit anythingSelectedChanged(); }

    signals:
        void selectAllChanged();
        void artworksCountChanged();
        void anythingSelectedChanged();

    public:
        void setupItem(Artworks::IMetadataOperator *item, Common::SuggestionFlags flags);
        void setupItems(std::vector<Artworks::IMetadataOperator *> &items, Common::SuggestionFlags flags);
#if defined(INTEGRATION_TESTS) || defined(CORE_TESTS)
        SpellSuggestionsItem *getItem(int i) const { return m_SuggestionsList.at(i).get(); }
#endif

    private:
        void setArtworks(Artworks::ArtworksSnapshot &&snapshot);
        SuggestionsVector createSuggestionsRequests(Common::SuggestionFlags flags,
                                                    std::vector<Artworks::IMetadataOperator *> &items);
        bool processFailedReplacements(const SuggestionsVector &failedReplacements) const;
        SuggestionsVector setupSuggestions(const SuggestionsVector &items);

    public:
        virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
        virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    protected:
        virtual QHash<int, QByteArray> roleNames() const override;

    private:
        void updateItems() const;

    private:
        std::vector<std::shared_ptr<SpellSuggestionsItem> > m_SuggestionsList;
        std::vector<Artworks::IMetadataOperator *> m_CheckedItems;
        SpellCheckerService &m_SpellCheckerService;
        Commands::AppMessages &m_Messages;
    };
}

#endif // SPELLCHECKSUGGESTIONMODEL_H

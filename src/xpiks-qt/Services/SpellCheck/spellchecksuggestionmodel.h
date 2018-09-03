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
#include <functional>
#include <Common/flags.h>
#include <Artworks/artworkssnapshot.h>

namespace Models {
    class ArtworksListModel;
}

namespace Artworks {
    class BasicMetadataModel;
}

namespace Services {
    class IArtworksUpdater;
}

namespace SpellCheck {
    class SpellSuggestionsItem;
    class SpellCheckService;
    class KeywordSpellSuggestions;
    class ISpellSuggestionsTarget;

    typedef std::vector<std::shared_ptr<SpellSuggestionsItem> > SuggestionsVector;
    typedef std::vector<std::shared_ptr<KeywordSpellSuggestions> > KeywordsSuggestionsVector;

    class SpellCheckSuggestionModel: public QAbstractListModel
    {
        Q_OBJECT
        Q_PROPERTY(int artworksCount READ getArtworksCount NOTIFY artworksCountChanged)
        Q_PROPERTY(bool anythingSelected READ getAnythingSelected NOTIFY anythingSelectedChanged)

    public:
        SpellCheckSuggestionModel(SpellCheckService &spellCheckerService);

    public:
        enum KeywordSpellSuggestions_Roles {
            WordRole = Qt::UserRole + 1,
            ReplacementIndexRole,
            ReplacementOriginRole
        };

    public:
        int getArtworksCount() const;
        bool getAnythingSelected() const;

    public:
        Q_INVOKABLE QObject *getSuggestionObject(int index) const;
        Q_INVOKABLE void clearModel();
        Q_INVOKABLE void submitCorrections() const;
        Q_INVOKABLE void resetAllSuggestions();
        Q_INVOKABLE void updateSelection() { emit anythingSelectedChanged(); }

    signals:
        void selectAllChanged();
        void artworksCountChanged();
        void anythingSelectedChanged();

    public:
        void setupModel(std::shared_ptr<ISpellSuggestionsTarget> const &target, Common::SpellCheckFlags flags);
#if defined(INTEGRATION_TESTS) || defined(CORE_TESTS)
        SpellSuggestionsItem *getItem(int i) const { return m_SuggestionsList.at(i).get(); }
#endif

    private:
        bool processFailedReplacements(const SuggestionsVector &failedReplacements) const;
        SuggestionsVector setupSuggestions(const SuggestionsVector &items);
        void setupRequests(const SuggestionsVector &requests);

    public:
        virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
        virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    protected:
        virtual QHash<int, QByteArray> roleNames() const override;

    private:
        std::vector<std::shared_ptr<SpellSuggestionsItem> > m_SuggestionsList;
        std::shared_ptr<ISpellSuggestionsTarget> m_SpellSuggestionsTarget;
        SpellCheckService &m_SpellCheckService;
    };
}

#endif // SPELLCHECKSUGGESTIONMODEL_H
/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef FINDANDREPLACEMODEL_H
#define FINDANDREPLACEMODEL_H

#include <memory>
#include <vector>

#include <QAbstractListModel>
#include <QHash>
#include <QModelIndex>
#include <QObject>
#include <QString>
#include <QVariant>
#include <Qt>

#include "Artworks/artworkssnapshot.h"
#include "Common/flags.h"
#include "Common/iflagsprovider.h"
#include "Common/messages.h"
#include "Connectivity/analyticsuserevent.h"
#include "Models/iactionmodel.h"

#ifdef INTEGRATION_TESTS
#include "Models/Editing/previewartworkelement.h"
#endif

class QByteArray;
class QModelIndex;
class QQuickTextDocument;

namespace Commands {
    class ICommandManager;
}

namespace QMLExtensions {
    class ColorsModel;
}

namespace Services {
    class IArtworksUpdater;
}

namespace Models {
    class PreviewArtworkElement;

    class FindAndReplaceModel:
            public QAbstractListModel,
            public Models::IActionModel,
            public Common::IFlagsProvider<Common::SearchFlags>,
            public Common::MessagesSource<Common::NamedType<Connectivity::EventType>>
    {
        Q_OBJECT
        Q_PROPERTY(QString replaceFrom READ getReplaceFrom WRITE setReplaceFrom NOTIFY replaceFromChanged)
        Q_PROPERTY(QString replaceTo READ getReplaceTo WRITE setReplaceTo NOTIFY replaceToChanged)
        Q_PROPERTY(bool searchInTitle READ getSearchInTitle WRITE setSearchInTitle NOTIFY searchInTitleChanged)
        Q_PROPERTY(bool searchInDescription READ getSearchInDescription WRITE setSearchInDescription NOTIFY searchInDescriptionChanged)
        Q_PROPERTY(bool searchInKeywords READ getSearchInKeywords WRITE setSearchInKeywords NOTIFY searchInKeywordsChanged)
        Q_PROPERTY(bool caseSensitive READ getCaseSensitive WRITE setCaseSensitive NOTIFY caseSensitiveChanged)
        Q_PROPERTY(bool searchWholeWords READ getSearchWholeWords WRITE setSearchWholeWords NOTIFY searchWholeWordsChanged)
        Q_PROPERTY(int count READ getArtworksCount NOTIFY countChanged)

    public:
        FindAndReplaceModel(QMLExtensions::ColorsModel &colorsModel,
                            Services::IArtworksUpdater &artworksUpdater,
                            QObject *parent=nullptr);

    public:
        virtual Common::SearchFlags getFlags() const override { return m_Flags; }
        const QString &getReplaceFrom() const{ return m_ReplaceFrom; }
        const QString &getReplaceTo() const { return m_ReplaceTo; }
        int getArtworksCount() const { return (int)m_PreviewElements.size(); }

        void setReplaceFrom(const QString &value);
        void setReplaceTo(const QString &value);
    public:
        bool getSearchInTitle() const {
            return Common::HasFlag(m_Flags, Common::SearchFlags::Title);
        }

        void setSearchInTitle(bool value);

        bool getSearchInDescription() const {
            return Common::HasFlag(m_Flags, Common::SearchFlags::Description);
        }

        void setSearchInDescription(bool value);

        bool getSearchInKeywords() const {
            return Common::HasFlag(m_Flags, Common::SearchFlags::Keywords);
        }

        void setSearchInKeywords(bool value);

        bool getCaseSensitive() const {
            return Common::HasFlag(m_Flags, Common::SearchFlags::CaseSensitive);
        }

        void setCaseSensitive(bool value);

        bool getSearchWholeWords() const {
            return Common::HasFlag(m_Flags, Common::SearchFlags::WholeWords);
        }

        void setSearchWholeWords(bool value);

    public:
        enum FindAndReplaceModel_Roles {
            PathRole = Qt::UserRole + 1,
            IsSelectedRole,
            EditIsSelectedRole,
            HasTitleMatchRole,
            HasDescriptionMatchRole,
            HasKeywordsMatchRole,
            IsVideoRole
        };

    public:
        void findReplaceCandidates(const Artworks::ArtworksSnapshot &snapshot);

    public:
#if !defined(CORE_TESTS) && !defined(INTEGRATION_TESTS)
        Q_INVOKABLE void initHighlighting(QQuickTextDocument *document);
#endif
        Q_INVOKABLE QString getSearchTitle(int index);
        Q_INVOKABLE QString getSearchDescription(int index);
        Q_INVOKABLE QString getSearchKeywords(int index);
        Q_INVOKABLE void selectAll() { setAllSelected(true); }
        Q_INVOKABLE void unselectAll() { setAllSelected(false); }
        Q_INVOKABLE bool anySearchDestination() const;

#ifdef INTEGRATION_TESTS
        void setItemSelected(int index, bool selected) {
            m_PreviewElements[index]->setIsSelected(selected);
        }
#endif

        // IActionModel interface
    public:
        virtual std::shared_ptr<Commands::ICommand> getActionCommand(bool yesno) override;
        virtual void resetModel() override;

        // QAbstractListModel interface
    public:
        virtual int rowCount(const QModelIndex &parent=QModelIndex()) const override;
        virtual QVariant data(const QModelIndex &index, int role=Qt::DisplayRole) const override;
        virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
        virtual bool setData(const QModelIndex &index, const QVariant &value, int role=Qt::EditRole) override;

    protected:
        virtual QHash<int, QByteArray> roleNames() const override;

    signals:
        void replaceFromChanged(const QString &value);
        void replaceToChanged(const QString &value);
        void searchInTitleChanged(bool value);
        void searchInDescriptionChanged(bool value);
        void searchInKeywordsChanged(bool value);
        void caseSensitiveChanged(bool value);
        void searchWholeWordsChanged(bool value);
        void countChanged(int value);
        void allSelectedChanged();

    private:
        void updatePreviewFlags();
        QString filterText(const QString &text);
        void setAllSelected(bool isSelected);
        void initDefaultFlags();
        void normalizeSearchCriteria();

    private:
        std::vector<std::shared_ptr<PreviewArtworkElement>> m_PreviewElements;
        QString m_ReplaceFrom;
        QString m_ReplaceTo;
        QMLExtensions::ColorsModel &m_ColorsModel;
        Services::IArtworksUpdater &m_ArtworksUpdater;
        Common::SearchFlags m_Flags;
    };
}
#endif // FINDANDREPLACEMODEL_H

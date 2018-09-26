/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef DELETEKEYWORDSVIEWMODEL_H
#define DELETEKEYWORDSVIEWMODEL_H

#include <QHash>
#include <Common/types.h>
#include <Artworks/basickeywordsmodel.h>
#include <Models/Artworks/artworksviewmodel.h>
#include <Models/iactionmodel.h>
#include <KeywordsPresets/ipresetsmanager.h>
#include <Common/messages.h>

namespace Commands {
    class ICommandManager;
}

namespace KeywordsPresets {
    class IPresetsManager;
}

namespace Models {
    using BasicSpellCheckMessageType = Common::NamedType<std::shared_ptr<Artworks::IBasicModelSource>, Common::MessageType::SpellCheck>;

    class DeleteKeywordsViewModel:
            public Models::ArtworksViewModel,
            public Models::IActionModel,
            public Common::MessagesSource<BasicSpellCheckMessageType>
    {
        Q_OBJECT
        Q_PROPERTY(int commonKeywordsCount READ getCommonKeywordsCount NOTIFY commonKeywordsCountChanged)
        Q_PROPERTY(int keywordsToDeleteCount READ getKeywordsToDeleteCount NOTIFY keywordsToDeleteCountChanged)
        Q_PROPERTY(bool caseSensitive READ getCaseSensitive WRITE setCaseSensitive NOTIFY caseSensitiveChanged)

    public:
        DeleteKeywordsViewModel(KeywordsPresets::IPresetsManager &presetsManager,
                                QObject *parent=nullptr);

    public:
        int getCommonKeywordsCount() { return m_CommonKeywordsModel.getKeywordsCount(); }
        int getKeywordsToDeleteCount() { return m_KeywordsToDeleteModel.getKeywordsCount(); }
        bool getCaseSensitive() const { return m_CaseSensitive; }
        void setCaseSensitive(bool value) {
            if (m_CaseSensitive != value) {
                m_CaseSensitive = value;
                emit caseSensitiveChanged();
            }
        }

    signals:
        void commonKeywordsCountChanged();
        void keywordsToDeleteCountChanged();
        void caseSensitiveChanged();

    public:
        virtual void setArtworks(Artworks::ArtworksSnapshot const &artworks) override;
        virtual bool removeUnavailableItems() override;

        // IActionModel interface
    public:
        virtual std::shared_ptr<Commands::ICommand> getActionCommand(bool yesno) override;
        virtual void resetModel() override;

#ifdef CORE_TESTS
    public:
        bool containsCommonKeyword(const QString &keyword) { return m_CommonKeywordsModel.containsKeyword(keyword); }
#endif

#if defined(UI_TESTS) || defined(INTEGRATION_TESTS)
    public:
        void clearModel();
        Q_INVOKABLE void appendCommonKeyword(const QString &keyword);
#endif

    protected:
        virtual bool doRemoveSelectedArtworks() override;

    public:
        Q_INVOKABLE QObject *getCommonKeywordsObject();
        Q_INVOKABLE QObject *getKeywordsToDeleteObject();

    public:
        Q_INVOKABLE void removeKeywordToDeleteAt(int keywordIndex);
        Q_INVOKABLE void removeLastKeywordToDelete();
        Q_INVOKABLE void clearKeywordsToDelete();
        Q_INVOKABLE QString removeCommonKeywordAt(int keywordIndex);
        Q_INVOKABLE void appendKeywordToDelete(const QString &keyword);
        Q_INVOKABLE void pasteKeywordsToDelete(const QStringList &keywords);
        Q_INVOKABLE bool addPreset(KeywordsPresets::ID_t presetID);

    private:
        void recombineKeywords();
        void fillKeywordsHash(QHash<QString, int> &keywordsHash);
        void submitForSpellCheck();

    private:
        Artworks::BasicKeywordsModel m_KeywordsToDeleteModel;
        Artworks::BasicKeywordsModel m_CommonKeywordsModel;
        KeywordsPresets::IPresetsManager &m_PresetsManager;
        bool m_CaseSensitive;
    };
}

#endif // DELETEKEYWORDSVIEWMODEL_H

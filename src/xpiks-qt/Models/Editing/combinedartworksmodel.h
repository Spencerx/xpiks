/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef COMBINEDARTWORKSMODEL_H
#define COMBINEDARTWORKSMODEL_H

#include <QAbstractListModel>
#include <QQmlEngine>
#include <QStringList>
#include <QString>
#include <QList>
#include <QSet>
#include <QQuickTextDocument>
#include <memory>
#include <vector>
#include <Models/Artworks/artworksviewmodel.h>
#include <Artworks/basicmetadatamodel.h>
#include <Common/flags.h>
#include <Services/SpellCheck/spellcheckiteminfo.h>
#include <Common/hold.h>
#include <Artworks/artworkelement.h>
#include "artworkproxybase.h"
#include <Common/delayedactionentity.h>
#include <Models/Editing/icurrenteditable.h>

namespace Commands {
    class ICommandManager;
    class AppMessages;
}

namespace KeywordsPresets {
    class IPresetsManager;
}

namespace AutoComplete {
    class ICompletionSource;
}

namespace Models {
    class CombinedArtworksModel:
            public ArtworksViewModel,
            public ArtworkProxyBase,
            public Common::DelayedActionEntity
    {
        Q_OBJECT
        Q_PROPERTY(QString description READ getDescription WRITE setDescription NOTIFY descriptionChanged)
        Q_PROPERTY(QString title READ getTitle WRITE setTitle NOTIFY titleChanged)
        Q_PROPERTY(bool hasTitleSpellErrors READ getHasTitleSpellErrors NOTIFY titleSpellingChanged)
        Q_PROPERTY(bool hasDescriptionSpellErrors READ getHasDescriptionSpellError NOTIFY descriptionSpellingChanged)
        Q_PROPERTY(bool hasKeywordsSpellErrors READ getHasKeywordsSpellError NOTIFY keywordsSpellingChanged)
        Q_PROPERTY(int keywordsCount READ getKeywordsCount NOTIFY keywordsCountChanged)
        Q_PROPERTY(bool changeDescription READ getChangeDescription WRITE setChangeDescription NOTIFY changeDescriptionChanged)
        Q_PROPERTY(bool changeTitle READ getChangeTitle WRITE setChangeTitle NOTIFY changeTitleChanged)
        Q_PROPERTY(bool changeKeywords READ getChangeKeywords WRITE setChangeKeywords NOTIFY changeKeywordsChanged)
        Q_PROPERTY(bool appendKeywords READ getAppendKeywords WRITE setAppendKeywords NOTIFY appendKeywordsChanged)

    public:
        CombinedArtworksModel(Commands::ICommandManager &commandManager,
                              KeywordsPresets::IPresetsManager &presetsManager,
                              AutoComplete::ICompletionSource &completionSource,
                              Commands::AppMessages &messages,
                              QObject *parent=0);

    protected:
        virtual void setArtworks(Artworks::ArtworksSnapshot &&artworks) override;

    private:
        enum CombinedEditModifiedFlags {
            FlagTitleModified = 1 << 0,
            FlagDescriptionModified = 1 << 1,
            FlagKeywordsModified = 1 << 2,
            FlagSpellingFixed = 1 << 3
        };

        inline bool getTitleModifiedFlag() const { return Common::HasFlag(m_ModifiedFlags, FlagTitleModified); }
        inline bool getDescriptionModifiedFlag() const { return Common::HasFlag(m_ModifiedFlags, FlagDescriptionModified); }
        inline bool getKeywordsModifiedFlag() const { return Common::HasFlag(m_ModifiedFlags, FlagKeywordsModified); }
        inline bool getSpellingFixedFlag() const { return Common::HasFlag(m_ModifiedFlags, FlagSpellingFixed); }

        inline void setTitleModifiedFlag(bool value) { Common::ApplyFlag(m_ModifiedFlags, value, FlagTitleModified); }
        inline void setDescriptionModifiedFlag(bool value) { Common::ApplyFlag(m_ModifiedFlags, value, FlagDescriptionModified); }
        inline void setKeywordsModifiedFlag(bool value) { Common::ApplyFlag(m_ModifiedFlags, value, FlagKeywordsModified); }
        inline void setSpellingFixedFlag(bool value) { Common::ApplyFlag(m_ModifiedFlags, value, FlagSpellingFixed); }

        void setTitleModified(bool value) { setTitleModifiedFlag(value); }
        void setDescriptionModified(bool value) { setDescriptionModifiedFlag(value); }
        void setKeywordsModified(bool value) { setKeywordsModifiedFlag(value); }
        void setSpellingFixed(bool value) { setSpellingFixedFlag(value); }

    public:
        bool isTitleModified() const { return getTitleModifiedFlag(); }
        bool isDescriptionModified() const { return getDescriptionModifiedFlag(); }
        bool areKeywordsModified() const { return getKeywordsModifiedFlag(); }
        bool isSpellingFixed() const { return getSpellingFixedFlag(); }

    private:
        void initKeywords(const QStringList &keywords) { m_CommonKeywordsModel.setKeywords(keywords); setKeywordsModified(false); }
        void initDescription(const QString &description) { setDescription(description); setDescriptionModified(false); }
        void initTitle(const QString &title) { setTitle(title); setTitleModified(false); }
        void recombineArtworks();

    public:
        virtual void setDescription(const QString &value) override;
        virtual void setTitle(const QString &value) override;
        virtual void setKeywords(const QStringList &keywords) override;

        bool getChangeDescription() const { return Common::HasFlag(m_EditFlags, Common::ArtworkEditFlags::EditDescription); }
        void setChangeDescription(bool value);

        bool getChangeTitle() const { return Common::HasFlag(m_EditFlags, Common::ArtworkEditFlags::EditTitle); }
        void setChangeTitle(bool value);

        bool getChangeKeywords() const { return Common::HasFlag(m_EditFlags, Common::ArtworkEditFlags::EditKeywords); }
        void setChangeKeywords(bool value);

        bool getAppendKeywords() const { return Common::HasFlag(m_EditFlags, Common::ArtworkEditFlags::AppendKeywords); }
        void setAppendKeywords(bool value);

    signals:
        void descriptionChanged();
        void titleChanged();
        void keywordsCountChanged();
        void changeDescriptionChanged();
        void changeKeywordsChanged();
        void changeTitleChanged();
        void appendKeywordsChanged();
        void artworkUnavailable(int index);
        void completionsAvailable();
        void editingPaused();
        void titleSpellingChanged();
        void descriptionSpellingChanged();
        void keywordsSpellingChanged();

    protected:
        virtual void signalDescriptionChanged() override { emit descriptionChanged(); }
        virtual void signalTitleChanged() override { emit titleChanged(); }
        virtual void signalKeywordsCountChanged() override { emit keywordsCountChanged(); }

    public:
        void generateAboutToBeRemoved();

#ifdef CORE_TESTS
        QStringList getKeywords();
#endif

    public:
        Q_INVOKABLE void editKeyword(int index, const QString &replacement);
        Q_INVOKABLE QString removeKeywordAt(int keywordIndex);
        Q_INVOKABLE void removeLastKeyword();
        Q_INVOKABLE bool moveKeyword(int from, int to);
        Q_INVOKABLE bool appendKeyword(const QString &keyword);
        Q_INVOKABLE void pasteKeywords(const QStringList &keywords);
        Q_INVOKABLE void saveEdits();
        Q_INVOKABLE void clearKeywords();

        Q_INVOKABLE QString getKeywordsString() { return m_CommonKeywordsModel.getKeywordsString(); }
        Q_INVOKABLE QObject *getBasicModel() {
            QObject *item = &m_CommonKeywordsModel;
            QQmlEngine::setObjectOwnership(item, QQmlEngine::CppOwnership);

            return item;
        }

        Q_INVOKABLE void suggestCorrections();
        Q_INVOKABLE void setupDuplicatesModel();
        Q_INVOKABLE void assignFromSelected();
        Q_INVOKABLE void plainTextEdit(const QString &rawKeywords, bool spaceIsSeparator=false);

        Q_INVOKABLE bool hasTitleWordSpellError(const QString &word);
        Q_INVOKABLE bool hasDescriptionWordSpellError(const QString &word);
        Q_INVOKABLE void expandPreset(int keywordIndex, unsigned int presetID);
        Q_INVOKABLE void expandLastKeywordAsPreset();
        Q_INVOKABLE void addPreset(unsigned int presetID);
        Q_INVOKABLE bool acceptCompletionAsPreset(int completionID);

    private:
        void enableAllFields();
        void assignFromOneArtwork();
        void assignFromManyArtworks();
        void recombineArtworks(std::function<bool (const Artworks::ArtworkElement *)> pred);
        bool findNonEmptyData(std::function<bool (const Artworks::ArtworkElement *)> pred, int &index,
                              Artworks::ArtworkMetadata *&artworkMetadata);

    public slots:
        void onDescriptionSpellingChanged();
        void onTitleSpellingChanged();
        void spellCheckErrorsFixedHandler();
        void userDictUpdateHandler(const QStringList &keywords, bool overwritten);
        void userDictClearedHandler();
        void onEditingPaused();

#ifdef INTEGRATION_TESTS
    public:
        Artworks::BasicMetadataModel *retrieveBasicMetadataModel() { return getBasicMetadataModel(); }
#endif

    protected:
        virtual Artworks::BasicMetadataModel *getBasicMetadataModel() override { return &m_CommonKeywordsModel; }
        virtual Artworks::IMetadataOperator *getMetadataOperator() override { return &m_CommonKeywordsModel; }

    protected:
        virtual void doJustEdited() override;
        virtual void submitForInspection() override;

    protected:
        virtual Common::ID_t getSpecialItemID() override;
        virtual bool doRemoveSelectedArtworks() override;
        virtual void doResetModel() override;

        // DelayedActionEntity implementation
    protected:
        virtual void doKillTimer(int timerId) override { this->killTimer(timerId); }
        virtual int doStartTimer(int interval, Qt::TimerType timerType) override { return this->startTimer(interval, timerType); }
        virtual void doOnTimer() override;
        virtual void timerEvent(QTimerEvent *event) override { onQtTimer(event); }
        virtual void callBaseTimer(QTimerEvent *event) override { ArtworksViewModel::timerEvent(event); }

    public:
        virtual bool removeUnavailableItems() override;

    private:
        Common::Hold m_HoldPlaceholder;
        Commands::ICommandManager &m_CommandManager;
        KeywordsPresets::IPresetsManager &m_PresetsManager;
        AutoComplete::ICompletionSource &m_CompletionSource;
        Commands::AppMessages &m_Messages;
        Artworks::BasicMetadataModel m_CommonKeywordsModel;
        SpellCheck::SpellCheckItemInfo m_SpellCheckInfo;
        Common::ArtworkEditFlags m_EditFlags;
        Common::flag_t m_ModifiedFlags;
    };
}

#endif // COMBINEDARTWORKSMODEL_H

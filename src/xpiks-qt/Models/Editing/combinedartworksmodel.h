/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef COMBINEDARTWORKSMODEL_H
#define COMBINEDARTWORKSMODEL_H

#include <functional>
#include <memory>

#include <QObject>
#include <QString>
#include <QStringList>
#include <Qt>

#include "Artworks/artworkssnapshot.h"
#include "Artworks/basicmetadatamodel.h"
#include "Artworks/iartworkmetadata.h"
#include "Artworks/ibasicmodelsource.h"
#include "Common/delayedactionentity.h"
#include "Common/flags.h"
#include "Common/messages.h"
#include "Common/types.h"
#include "Models/Artworks/artworksviewmodel.h"
#include "Models/Editing/artworkproxybase.h"
#include "Models/iactionmodel.h"
#include "Services/SpellCheck/spellcheckinfo.h"

class QTimerEvent;

namespace Artworks {
    class ArtworkElement;
    class ArtworkMetadata;
}

namespace Commands {
    class ICommand;
}

namespace KeywordsPresets {
    class IPresetsManager;
}

namespace AutoComplete {
    class ICompletionSource;
}

namespace Services {
    class IArtworksUpdater;
}

namespace Models {
    class ICurrentEditable;
    struct QuickBufferMessage;

    using BasicSpellCheckMessageType = Common::NamedType<std::shared_ptr<Artworks::IBasicModelSource>, Common::MessageType::SpellCheck>;

    class CombinedArtworksModel:
            public ArtworksViewModel,
            public ArtworkProxyBase,
            public Models::IActionModel,
            public Artworks::IBasicModelSource,
            public Common::DelayedActionEntity,
            public Common::MessagesSource<BasicSpellCheckMessageType>,
            public Common::MessagesSource<QuickBufferMessage>,
            public Common::MessagesSource<std::shared_ptr<ICurrentEditable>>
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

        using Common::MessagesSource<BasicSpellCheckMessageType>::sendMessage;
        using Common::MessagesSource<QuickBufferMessage>::sendMessage;
        using Common::MessagesSource<std::shared_ptr<ICurrentEditable>>::sendMessage;

    public:
        CombinedArtworksModel(Services::IArtworksUpdater &artworksUpdater,
                              KeywordsPresets::IPresetsManager &presetsManager,
                              QObject *parent=nullptr);

    public:
        virtual void setArtworks(Artworks::ArtworksSnapshot const &artworks) override;

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
        void initKeywords(const QStringList &keywords, bool onlyAppend);
        void initDescription(const QString &description, bool enabled);
        void initTitle(const QString &title, bool enabled);
        void recombineArtworks();
        void registerAsCurrentEditable();

    public:
        virtual Artworks::BasicMetadataModel &getBasicModel() override { return m_CommonKeywordsModel; }

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

#ifdef CORE_TESTS
    public:
        QStringList getKeywords();
#endif

    public:
        Q_INVOKABLE void editKeyword(int index, const QString &replacement);
        Q_INVOKABLE QString removeKeywordAt(int keywordIndex);
        Q_INVOKABLE void removeLastKeyword();
        Q_INVOKABLE bool moveKeyword(int from, int to);
        Q_INVOKABLE bool appendKeyword(const QString &keyword);
        Q_INVOKABLE void pasteKeywords(const QStringList &keywords);
        Q_INVOKABLE void clearKeywords();

        Q_INVOKABLE QString getKeywordsString() { return m_CommonKeywordsModel.getKeywordsString(); }
        Q_INVOKABLE QObject *getBasicModelObject();

        Q_INVOKABLE void assignFromSelected();

        Q_INVOKABLE bool hasTitleWordSpellError(const QString &word);
        Q_INVOKABLE bool hasDescriptionWordSpellError(const QString &word);
        Q_INVOKABLE void expandPreset(int keywordIndex, unsigned int presetID);
        Q_INVOKABLE void expandLastKeywordAsPreset();
        Q_INVOKABLE void addPreset(unsigned int presetID);
#ifdef UI_TESTS
        Q_INVOKABLE void clearModel();
        Q_INVOKABLE void setupModel() { registerAsCurrentEditable(); }
#endif

    public:
        virtual std::shared_ptr<Commands::ICommand> getActionCommand(bool yesno) override;
        virtual void resetModel() override;

    public:
        void copyToQuickBuffer();
        bool acceptCompletionAsPreset(AutoComplete::ICompletionSource &completionSource, int completionID);

    private:
        void assignFromOneArtwork();
        void assignFromManyArtworks();
        void recombineArtworks(std::function<bool (std::shared_ptr<Artworks::ArtworkElement> const &)> pred);
        bool findNonEmptyData(std::function<bool (std::shared_ptr<Artworks::ArtworkElement> const &)> pred, int &index,
                              std::shared_ptr<Artworks::ArtworkMetadata> &artworkMetadata);

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
        virtual Artworks::IArtworkMetadata *getArtworkMetadata() override { return &m_CommonKeywordsModel; }

    protected:
        virtual void doJustEdited() override;
        virtual void submitForInspection() override;

    protected:
        virtual Common::ID_t getSpecialItemID() override;
        virtual bool doRemoveSelectedArtworks() override;

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
        KeywordsPresets::IPresetsManager &m_PresetsManager;
        Services::IArtworksUpdater &m_ArtworksUpdater;
        Artworks::BasicMetadataModel m_CommonKeywordsModel;
        SpellCheck::SpellCheckInfo m_SpellCheckInfo;
        Common::ArtworkEditFlags m_EditFlags;
        Common::flag_t m_ModifiedFlags;
    };
}

#endif // COMBINEDARTWORKSMODEL_H

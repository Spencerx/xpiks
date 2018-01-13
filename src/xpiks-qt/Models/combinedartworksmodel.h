/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2017 Taras Kushnir <kushnirTV@gmail.com>
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
#include "artworksviewmodel.h"
#include "../Common/basicmetadatamodel.h"
#include "../Common/flags.h"
#include "../SpellCheck/spellcheckiteminfo.h"
#include "../Common/hold.h"
#include "../Models/artworkelement.h"
#include "artworkproxybase.h"
#include "../Common/delayedactionentity.h"

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
        CombinedArtworksModel(QObject *parent=0);

        virtual ~CombinedArtworksModel() {}

    public:
        virtual void setArtworks(MetadataIO::WeakArtworksSnapshot &artworks) override;

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
        void initKeywords(const QStringList &ek) { m_CommonKeywordsModel.setKeywords(ek); setKeywordsModified(false); }
        void initDescription(const QString &description) { setDescription(description); setDescriptionModified(false); }
        void initTitle(const QString &title) { setTitle(title); setTitleModified(false); }
        void recombineArtworks();

    public:
        virtual void acceptSuggestedKeywords(const QStringList &keywords);

    public:
        virtual void setDescription(const QString &value) override;
        virtual void setTitle(const QString &value) override;
        virtual void setKeywords(const QStringList &keywords) override;

        bool getChangeDescription() const { return Common::HasFlag(m_EditFlags, Common::CombinedEditFlags::EditDescription); }
        void setChangeDescription(bool value);

        bool getChangeTitle() const { return Common::HasFlag(m_EditFlags, Common::CombinedEditFlags::EditTitle); }
        void setChangeTitle(bool value);

        bool getChangeKeywords() const { return Common::HasFlag(m_EditFlags, Common::CombinedEditFlags::EditKeywords); }
        void setChangeKeywords(bool value);

        bool getAppendKeywords() const { return Common::HasFlag(m_EditFlags, Common::CombinedEditFlags::AppendKeywords); }
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
        Q_INVOKABLE void initDescriptionHighlighting(QQuickTextDocument *document);
        Q_INVOKABLE void initTitleHighlighting(QQuickTextDocument *document);
        Q_INVOKABLE void assignFromSelected();
        Q_INVOKABLE void plainTextEdit(const QString &rawKeywords, bool spaceIsSeparator=false);

        Q_INVOKABLE bool hasTitleWordSpellError(const QString &word);
        Q_INVOKABLE bool hasDescriptionWordSpellError(const QString &word);
        Q_INVOKABLE void expandPreset(int keywordIndex, unsigned int presetID);
        Q_INVOKABLE void expandLastKeywordAsPreset();
        Q_INVOKABLE void addPreset(unsigned int presetID);
        Q_INVOKABLE void initSuggestion();
        Q_INVOKABLE void registerAsCurrentItem();
        Q_INVOKABLE void copyToQuickBuffer();
        Q_INVOKABLE void generateCompletions(const QString &prefix);
        Q_INVOKABLE bool acceptCompletionAsPreset(int completionID);

    private:
        void processCombinedEditCommand();
        void enableAllFields();
        void assignFromOneArtwork();
        void assignFromManyArtworks();
        void recombineArtworks(std::function<bool (const ArtworkElement *)> pred);
        bool findNonEmptyData(std::function<bool (const ArtworkElement *)> pred, int &index, ArtworkMetadata *&artworkMetadata);

    public slots:
        void onDescriptionSpellingChanged();
        void onTitleSpellingChanged();
        void spellCheckErrorsFixedHandler();

    public slots:
        void userDictUpdateHandler(const QStringList &keywords, bool overwritten);
        void userDictClearedHandler();
        void onEditingPaused();

#ifdef INTEGRATION_TESTS
    public:
        Common::BasicMetadataModel *retrieveBasicMetadataModel() { return getBasicMetadataModel(); }
#endif

    protected:
        virtual Common::BasicMetadataModel *getBasicMetadataModel() override { return &m_CommonKeywordsModel; }
        virtual Common::IMetadataOperator *getMetadataOperator() override { return &m_CommonKeywordsModel; }

    protected:
        virtual void doJustEdited() override;

    protected:
        virtual qint64 getSpecialItemID() override;
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
        Common::BasicMetadataModel m_CommonKeywordsModel;
        SpellCheck::SpellCheckItemInfo m_SpellCheckInfo;
        Common::CombinedEditFlags m_EditFlags;
        Common::flag_t m_ModifiedFlags;
    };
}

#endif // COMBINEDARTWORKSMODEL_H

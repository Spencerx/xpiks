/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef IMAGEMETADATA_H
#define IMAGEMETADATA_H

#include <QAbstractListModel>
#include <QReadWriteLock>
#include <QStringList>
#include <QFileInfo>
#include <QString>
#include <QVector>
#include <QSet>
#include <QMutex>
#include <memory>
#include "basicmetadatamodel.h"
#include "iartworkmetadata.h"
#include "ibasicmodelsource.h"
#include <Common/flags.h>
#include <Common/types.h>
#include <Common/delayedactionentity.h>
#include <Services/SpellCheck/spellcheckinfo.h>
#include <Services/SpellCheck/ispellcheckable.h>
#include <UndoRedo/artworkmetadatabackup.h>

namespace MetadataIO {
    struct CachedArtwork;
    struct OriginalMetadata;
}

class QTextDocument;
class QTimerEvent;

namespace Artworks {
    class SettingsModel;

    class ArtworkMetadata:
            public QObject,
            public Common::DelayedActionEntity,
            public Artworks::IArtworkMetadata,
            public Artworks::IBasicModelSource,
            public SpellCheck::ISpellCheckable,
            public std::enable_shared_from_this<ArtworkMetadata>
    {
        Q_OBJECT

    public:
        ArtworkMetadata(const QString &filepath, Common::ID_t ID, qint64 directoryID);

    public:
        enum FocusRequestType {
            FocusFromPrev,
            FocusFromNext
        };

    private:
        enum MetadataFlags {
            FlagIsModified = 1 << 0,
            FlagsIsSelected = 1 << 1,
            FlagIsInitialized = 1 << 2, // is initialized from real file
            FlagIsAlmostInitialized = 1 << 3, // is initialized from cached storage
            FlagIsUnavailable = 1 << 4,
            FlagIsLockedForEditing = 1 << 5,
            FlagIsLockedIO = 1 << 6,
            FlagIsReimportPending = 1 << 7,
            FlagIsReadOnly = 1 << 8,
            FlagIsRemoved = 1 << 9
        };

#define PROTECT_FLAGS_READ QReadLocker rlocker(&m_FlagsLock); Q_UNUSED(rlocker);
#define PROTECT_FLAGS_WRITE QWriteLocker wlocker(&m_FlagsLock); Q_UNUSED(wlocker);

        inline bool getIsModifiedFlag() { PROTECT_FLAGS_READ; return Common::HasFlag(m_MetadataFlags, FlagIsModified); }
        inline bool getIsSelectedFlag() { PROTECT_FLAGS_READ; return Common::HasFlag(m_MetadataFlags, FlagsIsSelected); }
        inline bool getIsUnavailableFlag() { PROTECT_FLAGS_READ; return Common::HasFlag(m_MetadataFlags, FlagIsUnavailable); }
        inline bool getIsInitializedFlag() { PROTECT_FLAGS_READ; return Common::HasFlag(m_MetadataFlags, FlagIsInitialized); }
        inline bool getIsAlmostInitializedFlag() { PROTECT_FLAGS_READ; return Common::HasFlag(m_MetadataFlags, FlagIsAlmostInitialized); }
        inline bool getIsLockedForEditingFlag() { PROTECT_FLAGS_READ; return Common::HasFlag(m_MetadataFlags, FlagIsLockedForEditing); }
        inline bool getIsLockedIOFlag() { PROTECT_FLAGS_READ; return Common::HasFlag(m_MetadataFlags, FlagIsLockedIO); }
        inline bool getIsReimportPendingFlag() { PROTECT_FLAGS_READ; return Common::HasFlag(m_MetadataFlags, FlagIsReimportPending); }
        inline bool getIsReadOnlyFlag() { PROTECT_FLAGS_READ; return Common::HasFlag(m_MetadataFlags, FlagIsReadOnly); }
        inline bool getIsRemovedFlag() { PROTECT_FLAGS_READ; return Common::HasFlag(m_MetadataFlags, FlagIsRemoved); }

        inline void setIsModifiedFlag(bool value) { PROTECT_FLAGS_WRITE; Common::ApplyFlag(m_MetadataFlags, value, FlagIsModified); }
        inline void setIsSelectedFlag(bool value) { PROTECT_FLAGS_WRITE; Common::ApplyFlag(m_MetadataFlags, value, FlagsIsSelected); }
        inline void setIsUnavailableFlag(bool value) { PROTECT_FLAGS_WRITE; Common::ApplyFlag(m_MetadataFlags, value, FlagIsUnavailable); }
        inline void setIsInitializedFlag(bool value) { PROTECT_FLAGS_WRITE; Common::ApplyFlag(m_MetadataFlags, value, FlagIsInitialized); }
        inline void setIsAlmostInitializedFlag(bool value) { PROTECT_FLAGS_WRITE; Common::ApplyFlag(m_MetadataFlags, value, FlagIsAlmostInitialized); }
        inline void setIsLockedForEditingFlag(bool value) { PROTECT_FLAGS_WRITE; Common::ApplyFlag(m_MetadataFlags, value, FlagIsLockedForEditing); }
        inline void setIsLockedIOFlag(bool value) { PROTECT_FLAGS_WRITE; Common::ApplyFlag(m_MetadataFlags, value, FlagIsLockedIO); }
        inline void setIsReimportPendingFlag(bool value) { PROTECT_FLAGS_WRITE; Common::ApplyFlag(m_MetadataFlags, value, FlagIsReimportPending); }
        inline void setIsReadOnlyFlag(bool value) { PROTECT_FLAGS_WRITE; Common::ApplyFlag(m_MetadataFlags, value, FlagIsReadOnly); }
        inline void setIsRemovedFlag(bool value) { PROTECT_FLAGS_WRITE; Common::ApplyFlag(m_MetadataFlags, value, FlagIsRemoved); }

#undef PROTECT_FLAGS_READ
#undef PROTECT_FLAGS_WRITE

    public:
        // exists only because of onArtworkEditingPaused() in ArtworksListModel
        // where we only receive raw ptr from Qt signal-slot sender() method
        std::shared_ptr<ArtworkMetadata> getptr() { return shared_from_this(); }

    public:
        void prepareForReimport();
        bool initFromOrigin(const MetadataIO::OriginalMetadata &originalMetadata, bool overwrite=false);
        bool initFromStorage(const MetadataIO::CachedArtwork &cachedArtwork);
        // called when Close is pressed in the Import dialog
        void initAsEmpty(const MetadataIO::OriginalMetadata &originalMetadata);
        void initAsEmpty();

    private:
        bool initFromOriginBeforeStorageUnsafe(const MetadataIO::OriginalMetadata &originalMetadata);
        bool initFromOriginAfterStorageUnsafe(const MetadataIO::OriginalMetadata &originalMetadata);
        bool initFromStorageBeforeOriginUnsafe(const MetadataIO::CachedArtwork &cachedArtwork);
        bool initFromStorageAfterOriginUnsafe(const MetadataIO::CachedArtwork &cachedArtwork);

    protected:
        virtual bool initFromOriginUnsafe(const MetadataIO::OriginalMetadata &originalMetadata) { Q_UNUSED(originalMetadata); return false; }
        virtual bool initFromStorageUnsafe(const MetadataIO::CachedArtwork &cachedArtwork) { Q_UNUSED(cachedArtwork); return false; }

    public:
        const QString &getFilepath() const { return m_ArtworkFilepath; }
        virtual const QString &getThumbnailPath() const { return m_ArtworkFilepath; }
        virtual QString getDirectory() const;
        QString getBaseFilename() const;
        bool isInDirectory(const QString &directoryAbsolutePath) const;

        bool isReadOnly() { return getIsReadOnlyFlag(); }
        bool isModified() { return getIsModifiedFlag(); }
        bool isSelected() { return getIsSelectedFlag(); }
        bool isUnavailable() { return getIsUnavailableFlag(); }
        bool isInitialized() { return getIsInitializedFlag(); }
        bool isAlmostInitialized() { return getIsAlmostInitializedFlag(); }
        bool isRemoved() { return getIsRemovedFlag(); }
        size_t getLastKnownIndex() const { return m_LastKnownIndex; }
        virtual qint64 getFileSize() const { return m_FileSize; }
        Common::ID_t const &getItemID() const { return m_ID; }
        bool hasDuplicates();

    public:
        void setCurrentIndex(size_t index) { m_LastKnownIndex = index; }
        Common::WarningFlags getWarningsFlags() const { return m_WarningsFlags; }
        void setWarningsFlags(Common::WarningFlags flags) { m_WarningsFlags = flags; }

    public:
        BasicMetadataModel const &getBasicMetadataModel() const { return m_MetadataModel; }
        BasicMetadataModel &getBasicMetadataModel() { return m_MetadataModel; }
        // IBasicModelSource interface
        virtual BasicKeywordsModel &getBasicModel() override { return m_MetadataModel; }

        bool isLockedForEditing() { return getIsLockedForEditingFlag(); }
        void setIsLockedForEditing(bool value) { setIsLockedForEditingFlag(value); }

        bool isLockedIO() { return getIsLockedIOFlag(); }
        void setIsLockedIO(bool value) { setIsLockedIOFlag(value); }

        virtual void clearModel();
        virtual bool clearKeywords() override;
        virtual bool editKeyword(size_t index, const QString &replacement) override;
        virtual bool replace(const QString &replaceWhat, const QString &replaceTo, Common::SearchFlags flags);
        virtual bool moveKeyword(size_t from, size_t to) override;

    public:
        virtual bool setDescription(const QString &value) override;
        virtual bool setTitle(const QString &value) override;
        virtual void setKeywords(const QStringList &keywords) override;

        bool setIsSelected(bool value);

        void invertSelection() { setIsSelected(!getIsSelectedFlag()); }

        void resetSelected() {
            if (getIsSelectedFlag()) {
                setIsSelectedFlag(false);
            }
        }

        void setFileSize(qint64 size) { m_FileSize = size; }

    public:
        bool areKeywordsEmpty() { return m_MetadataModel.areKeywordsEmpty(); }
        virtual bool removeKeywordAt(size_t index, QString &removed) override;
        virtual bool removeLastKeyword(QString &removed) override;
        virtual bool appendKeyword(const QString &keyword) override;
        virtual size_t appendKeywords(const QStringList &keywordsList) override;
        virtual bool removeKeywords(const QSet<QString> &keywordsSet, bool caseSensitive=true) override;
        virtual QString getKeywordsString() override { return m_MetadataModel.getKeywordsString(); }

    public:
        virtual Common::KeywordReplaceResult fixKeywordSpelling(size_t index, const QString &existing, const QString &replacement) override;
        virtual bool fixDescriptionSpelling(const QString &word, const QString &replacement) override;
        virtual bool fixTitleSpelling(const QString &word, const QString &replacement) override;
        virtual std::vector<KeywordItem> retrieveMisspelledKeywords() override;
        virtual QStringList retrieveMisspelledTitleWords() override;
        virtual QStringList retrieveMisspelledDescriptionWords() override;
        virtual bool processFailedKeywordReplacements(const std::vector<std::shared_ptr<SpellCheck::KeywordSpellSuggestions> > &candidatesForRemoval) override;
        virtual void afterReplaceCallback() override;

    public:
        QStringList getKeywords() { return m_MetadataModel.getKeywords(); }
        bool isEmpty() { return m_MetadataModel.isEmpty(); }
        QString getDescription() { return m_MetadataModel.getDescription(); }
        QString getTitle() { return m_MetadataModel.getTitle(); }

   public:
        virtual qint64 getDirectoryID() const { return m_DirectoryID; }

    public:
        void markModified();
        void setUnavailable() { setIsUnavailableFlag(true); }
        void setRemoved() { setIsRemovedFlag(true); }
        void resetRemoved() { setIsRemovedFlag(false); }
        void resetModified() { setIsModifiedFlag(false); }
        void requestFocus(FocusRequestType requestType);
        virtual bool expandPreset(size_t keywordIndex, const QStringList &presetList) override;
        virtual bool appendPreset(const QStringList &presetList) override;
        virtual bool hasKeywords(const QStringList &keywordsList) override;
        void deepDisconnect();
        void clearSpellingInfo();
        void resetSpellingInfo();
        void resetDuplicatesInfo();

#ifdef INTEGRATION_TESTS
    public:
        bool hasDuplicates(size_t index) { return m_MetadataModel.hasDuplicateAt(index); }
#endif

#ifdef CORE_TESTS
    public:
        SpellCheck::SpellCheckItemInfo *getSpellCheckInfo() { return &m_SpellCheckInfo; }
#endif

#ifndef CORE_TESTS
    private:
#else
    public:
#endif
        void setModified() { setIsModifiedFlag(true); }
        friend class UndoRedo::ArtworkMetadataBackup;

    signals:
        void modifiedChanged(bool newValue);
        void selectedChanged(bool newValue);
        void focusRequested(int directionSign);
        void editingPaused();
        void aboutToBeRemoved();
        void thumbnailUpdated();

    protected:
        virtual void resetFlags() { m_MetadataFlags = 0; }

        // DelayedActionEntity implementation
    protected:
        virtual void doKillTimer(int timerId) override { this->killTimer(timerId); }
        virtual int doStartTimer(int interval, Qt::TimerType timerType) override { return this->startTimer(interval, timerType); }
        virtual void doOnTimer() override;
        virtual void timerEvent(QTimerEvent *event) override { onQtTimer(event); }
        virtual void callBaseTimer(QTimerEvent *event) override { QObject::timerEvent(event); }

    private:
        SpellCheck::SpellCheckInfo m_SpellCheckInfo;
        BasicMetadataModel m_MetadataModel;
        QReadWriteLock m_FlagsLock;
        QMutex m_InitMutex;
        qint64 m_FileSize;  // in bytes
        QString m_ArtworkFilepath;
        Common::ID_t m_ID;
        qint64 m_DirectoryID;
        volatile Common::flag_t m_MetadataFlags;
        volatile size_t m_LastKnownIndex; // optimistic guess on current index of this item in artitemsmodel
        volatile Common::WarningFlags m_WarningsFlags;
    };
}

Q_DECLARE_METATYPE(Artworks::ArtworkMetadata *)

#endif // IMAGEMETADATA_H

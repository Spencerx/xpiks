/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef ARTWORKPROXYMODEL_H
#define ARTWORKPROXYMODEL_H

#include <QObject>
#include <QAbstractListModel>
#include <QStringList>
#include <QString>
#include <QList>
#include <QSet>
#include <QSize>
#include <memory>
#include <vector>
#include <Common/flags.h>
#include <Common/messages.h>
#include "artworkproxybase.h"
#include <Models/Editing/icurrenteditable.h>
#include <Models/keyvaluelist.h>

namespace Commands {
    class ICommandManager;
}

namespace KeywordsPresets {
    class IPresetsManager;
}

namespace AutoComplete {
    class ICompletionSource;
}

namespace Artworks {
    class ArtworkMetadata;
    class BasicMetadataModel;
    class VideoArtwork;
}

namespace Services {
    class IArtworksUpdater;
}

namespace Models {
    using BasicSpellCheckMessageType = Common::NamedType<std::shared_ptr<Artworks::IBasicModelSource>, Common::MessageType::SpellCheck>;

    class ArtworkProxyModel:
            public QObject,
            public ArtworkProxyBase,
            public Common::MessagesSource<BasicSpellCheckMessageType>,
            public Common::MessagesSource<std::shared_ptr<ICurrentEditable>>,
            public Common::MessagesSource<std::shared_ptr<Artworks::VideoArtwork>>
    {
        Q_OBJECT
        Q_PROPERTY(QString description READ getDescription WRITE setDescription NOTIFY descriptionChanged)
        Q_PROPERTY(QString title READ getTitle WRITE setTitle NOTIFY titleChanged)
        Q_PROPERTY(bool hasTitleSpellErrors READ getHasTitleSpellErrors NOTIFY titleSpellingChanged)
        Q_PROPERTY(bool hasDescriptionSpellErrors READ getHasDescriptionSpellError NOTIFY descriptionSpellingChanged)
        Q_PROPERTY(bool hasKeywordsSpellErrors READ getHasKeywordsSpellError NOTIFY keywordsSpellingChanged)
        Q_PROPERTY(int keywordsCount READ getKeywordsCount NOTIFY keywordsCountChanged)
        Q_PROPERTY(QString thumbPath READ getThumbPath NOTIFY thumbnailChanged)
        Q_PROPERTY(QString filePath READ getFilePath NOTIFY imagePathChanged)
        Q_PROPERTY(QString basename READ getBasename NOTIFY imagePathChanged)
        Q_PROPERTY(QString attachedVectorPath READ getAttachedVectorPath NOTIFY imagePathChanged)
        Q_PROPERTY(QSize imageSize READ retrieveImageSize NOTIFY imagePathChanged)
        Q_PROPERTY(QString fileSize READ retrieveFileSize NOTIFY imagePathChanged)
        Q_PROPERTY(QString dateTaken READ getDateTaken NOTIFY imagePathChanged)
        Q_PROPERTY(bool isVideo READ getIsVideo NOTIFY imagePathChanged)
        Q_PROPERTY(bool isValid READ getIsValid NOTIFY isValidChanged)
        Q_PROPERTY(int artworkIndex READ getArtworkIndex NOTIFY artworkChanged)

        using Common::MessagesSource<BasicSpellCheckMessageType>::sendMessage;
        using Common::MessagesSource<std::shared_ptr<ICurrentEditable>>::sendMessage;
        using Common::MessagesSource<std::shared_ptr<Artworks::VideoArtwork>>::sendMessage;

    public:
        explicit ArtworkProxyModel(Commands::ICommandManager &commandManager,
                                   KeywordsPresets::IPresetsManager &presetsManager,
                                   Services::IArtworksUpdater &artworksUpdater,
                                   QObject *parent = 0);
        virtual ~ArtworkProxyModel();

    public:
        bool getIsValid() const { return m_ArtworkMetadata != nullptr; }
        bool getIsVideo() const;
        QString getThumbPath() const;
        const QString &getFilePath() const;
        QString getBasename() const;
        int getArtworkIndex() const;

    public:
        virtual void setDescription(const QString &description) override;
        virtual void setTitle(const QString &title) override;
        virtual void setKeywords(const QStringList &keywords) override;

    public:
        std::shared_ptr<Artworks::ArtworkMetadata> const &getArtwork() const { return m_ArtworkMetadata; }

    signals:
        void imagePathChanged();
        void thumbnailChanged();
        void descriptionChanged();
        void titleChanged();
        void keywordsCountChanged();
        void itemBecomeUnavailable();
        void warningsCouldHaveChanged(size_t originalIndex);
        void duplicatesCouldHaveChanged(size_t originalIndex);
        void isValidChanged();
        void titleSpellingChanged();
        void descriptionSpellingChanged();
        void keywordsSpellingChanged();
        void artworkChanged();

    protected:
        virtual void signalDescriptionChanged() override { emit descriptionChanged(); }
        virtual void signalTitleChanged() override { emit titleChanged(); }
        virtual void signalKeywordsCountChanged() override { emit keywordsCountChanged(); }

    public slots:
        void userDictUpdateHandler(const QStringList &keywords, bool overwritten);
        void userDictClearedHandler();
        void afterSpellingErrorsFixedHandler();
        void onDescriptionSpellingChanged();
        void onTitleSpellingChanged();
        void itemUnavailableHandler(size_t index);

    public:
        Q_INVOKABLE void updateKeywords() { signalKeywordsCountChanged(); }
        Q_INVOKABLE void editKeyword(int index, const QString &replacement);
        Q_INVOKABLE void removeKeywordAt(int keywordIndex);
        Q_INVOKABLE void removeLastKeyword();
        Q_INVOKABLE bool moveKeyword(int from, int to);
        Q_INVOKABLE bool appendKeyword(const QString &keyword);
        Q_INVOKABLE void pasteKeywords(const QStringList &keywords);
        Q_INVOKABLE void clearKeywords();
        Q_INVOKABLE QString getKeywordsString();
        Q_INVOKABLE void plainTextEdit(const QString &rawKeywords, bool spaceIsSeparator=false);
        Q_INVOKABLE bool hasTitleWordSpellError(const QString &word);
        Q_INVOKABLE bool hasDescriptionWordSpellError(const QString &word);
        // --
        void setSourceArtwork(const std::shared_ptr<Artworks::ArtworkMetadata> &artwork);
        // --
        Q_INVOKABLE void resetModel();
        Q_INVOKABLE QObject *getBasicModelObject();
        Q_INVOKABLE QObject *getPropertiesMap();
        Q_INVOKABLE QSize retrieveImageSize() const;
        Q_INVOKABLE QString retrieveFileSize() const;
        Q_INVOKABLE QString getDateTaken() const;
        Q_INVOKABLE QString getAttachedVectorPath() const;
        Q_INVOKABLE void expandPreset(int keywordIndex, unsigned int presetID);
        Q_INVOKABLE void expandLastKeywordAsPreset();
        Q_INVOKABLE void addPreset(unsigned int presetID);
#ifdef UI_TESTS
        Q_INVOKABLE void clearModel();
#endif

    public:
        bool acceptCompletionAsPreset(AutoComplete::ICompletionSource &completionSource, int completionID);

    protected:
        virtual Artworks::BasicMetadataModel *getBasicMetadataModel() override;
        virtual Artworks::IArtworkMetadata *getArtworkMetadata() override;
        virtual Common::ID_t getSpecialItemID() override;
        virtual void submitForInspection() override;

    private:
        void connectArtworkSignals(Artworks::ArtworkMetadata *artwork);
        void updateModelProperties();
        void updateCurrentArtwork();
        void disconnectCurrentArtwork();
        void releaseCurrentArtwork();

    private:
        ArtworkPropertiesMap m_PropertiesMap;
        std::shared_ptr<Artworks::ArtworkMetadata> m_ArtworkMetadata;
        Commands::ICommandManager &m_CommandManager;
        KeywordsPresets::IPresetsManager &m_PresetsManager;
        Services::IArtworksUpdater &m_ArtworksUpdater;
    };
}

#endif // ARTWORKPROXYMODEL_H

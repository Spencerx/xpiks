/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef PRESETKEYWORDSMODEL_H
#define PRESETKEYWORDSMODEL_H

#include <atomic>
#include <cstddef>
#include <functional>
#include <memory>
#include <vector>

#include <QAbstractListModel>
#include <QHash>
#include <QModelIndex>
#include <QObject>
#include <QReadWriteLock>
#include <QSortFilterProxyModel>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QVariant>
#include <Qt>

#include "Common/delayedactionentity.h"
#include "Common/messages.h"
#include "Common/types.h"
#include "KeywordsPresets/ipresetsmanager.h"
#include "KeywordsPresets/presetgroupsmodel.h"
#include "KeywordsPresets/presetkeywordsmodelconfig.h"
#include "KeywordsPresets/presetmodel.h"

class QByteArray;
class QModelIndex;
class QTimerEvent;
template <class T> class QVector;
template <class T1, class T2> struct QPair;

namespace Artworks {
    class IBasicModelSource;
}

namespace Common {
    class ISystemEnvironment;
}

namespace KeywordsPresets {
    using BasicSpellCheckMessageType = Common::NamedType<std::shared_ptr<Artworks::IBasicModelSource>, Common::MessageType::SpellCheck>;

    class PresetKeywordsModel:
            public QAbstractListModel,
            public Common::DelayedActionEntity,
            public IPresetsManager,
            public Common::MessagesSource<BasicSpellCheckMessageType>
    {
        Q_OBJECT

    public:
        PresetKeywordsModel(Common::ISystemEnvironment &environment,
                            QObject *parent=nullptr);
        virtual ~PresetKeywordsModel();

#ifdef INTEGRATION_TESTS
    public:
        PresetKeywordsModelConfig &getKeywordsModelConfig() { return m_PresetsConfig; }
        void reload() { loadModelFromConfig(); }
#endif

    public:
        int getPresetsCount() const { return (int)m_PresetsList.size(); }
        PresetGroupsModel *getGroupsModel() { return &m_GroupsModel; }

    public:
        void initializePresets();
        bool tryGetNameFromIndex(int index, QString &name);
        bool tryGetGroupFromIndex(int index, int &groupID);

#if defined(CORE_TESTS) || defined(INTEGRATION_TESTS) || defined(UI_TESTS)
    public:
        void setName(int presetIndex, const QString &name);
        ID_t addItem(const QString &presetName, const QStringList &keywords);
        Q_INVOKABLE void clearModel();
        Q_INVOKABLE QString getKeywordsString(int presetIndex);
        bool removePresetByID(ID_t id);
#endif

    public:
        // IPresetsManager implementation
        virtual bool tryGetPreset(ID_t id, QStringList &keywords) override;
        virtual bool tryFindSinglePresetByName(const QString &name, bool strictMatch, ID_t &id) override;
        virtual void findPresetsByName(const QString &name, QVector<QPair<int, QString> > &results) override;
        virtual void findOrRegisterPreset(const QString &name, const QStringList &keywords, ID_t &id) override;
        virtual void addOrUpdatePreset(const QString &name, const QStringList &keywords, ID_t &id, bool &isAdded) override;
        virtual bool setPresetGroup(ID_t presetID, int groupID) override;
        virtual void findOrRegisterGroup(const QString &groupName, int &groupID) override;
        virtual void requestBackup() override;
        virtual void refreshPresets() override;
        virtual void triggerPresetsUpdated() override;

    public:
        bool tryFindPresetByFullName(const QString &name, bool caseSensitive, ID_t &id);
        void foreachPreset(const std::function<bool (size_t, std::shared_ptr<PresetModel> const &)> &action);

        // safe and unsafe versions exist because of plugins which
        // can use presets manager in multithreaded way
    private:
        // TODO: fix this "friendship" when C++ will be an adequate language
        // and will allow access to protected methods from derived classes
        friend class FilteredPresetsModelBase;
        friend class FilteredPresetKeywordsModel;
        friend class PresetKeywordsGroupModel;
        bool tryGetGroupFromIndexUnsafe(size_t index, int &groupID);
        bool tryGetNameFromIndexUnsafe(size_t index, QString &name);
        bool tryGetPresetUnsafe(size_t presetIndex, QStringList &keywords);
        bool tryGetPresetIndexByIDUnsafe(ID_t id, size_t &index);
        bool tryFindSinglePresetByNameUnsafe(const QString &name, bool strictMatch, size_t &index);
        void findPresetsByNameUnsafe(const QString &name, QVector<QPair<int, QString> > &results);
        void findOrRegisterPresetUnsafe(const QString &name, const QStringList &keywords, size_t &index);
        void addOrUpdatePresetUnsafe(const QString &name, const QStringList &keywords, size_t &index, bool &isAdded);
        bool tryFindPresetByFullNameUnsafe(const QString &name, bool caseSensitive, size_t &index);
        bool removeItemUnsafe(size_t index);
        ID_t getPresetID(size_t index);
        void checkSpelling(std::shared_ptr<PresetModel> const &preset);

    private:
        enum PresetKeywords_Roles {
            NameRole = Qt::UserRole + 1,
            EditNameRole,
            KeywordsCountRole,
            KeywordsStringRole,
            GroupRole,
            EditGroupRole,
            IdRole,
            IsNameValidRole
        };

    signals:
        void presetsUpdated();
        void backupRequest();

    public:
        virtual int rowCount(const QModelIndex &parent=QModelIndex()) const override;
        virtual QVariant data(const QModelIndex &index, int role=Qt::DisplayRole) const override;
        virtual bool setData(const QModelIndex &index, const QVariant &value, int role=Qt::EditRole) override;
        virtual Qt::ItemFlags flags(const QModelIndex &index) const override;

    protected:
        virtual QHash<int, QByteArray> roleNames() const override;
        virtual void removeInnerItem(int row);

    public:
        Q_INVOKABLE void removeItem(int row);
        Q_INVOKABLE void addItem();
        Q_INVOKABLE void editKeyword(int index, int keywordIndex, const QString &replacement);
        Q_INVOKABLE void removeKeywordAt(int index, int keywordIndex);
        Q_INVOKABLE void removeLastKeyword(int index);
        Q_INVOKABLE bool appendKeyword(int index, const QString &keyword);
        Q_INVOKABLE void pasteKeywords(int index, const QStringList &keywords);
        Q_INVOKABLE void plainTextEdit(int index, const QString &rawKeywords, bool spaceIsSeparator);
        Q_INVOKABLE QObject *getKeywordsModelObject(int index);
        Q_INVOKABLE void saveToConfig();
        Q_INVOKABLE void makeTitleValid(int row);

    private:
        bool makeTitleValidUnsafe(int row);
        /*Q_INVOKABLE*/ void loadModelFromConfig();

    public slots:
        void onBackupRequested();

    private slots:
        void onSavingTimerTriggered();

    private:
        void doLoadFromConfig();
        void removeAllPresets();
        int generateNextID();
        bool hasDuplicateNamesUnsafe(const QString &presetName, size_t index);

        // DelayedActionEntity implementation
    protected:
        virtual void doKillTimer(int timerId) override { this->killTimer(timerId); }
        virtual int doStartTimer(int interval, Qt::TimerType timerType) override { return this->startTimer(interval, timerType); }
        virtual void doOnTimer() override { emit requestBackup(); }
        virtual void timerEvent(QTimerEvent *event) override { onQtTimer(event); }
        virtual void callBaseTimer(QTimerEvent *event) override { QAbstractListModel::timerEvent(event); }

    private:
        Common::ISystemEnvironment &m_Environment;
        PresetKeywordsModelConfig m_PresetsConfig;
        PresetGroupsModel m_GroupsModel;
        QReadWriteLock m_PresetsLock;
        std::vector<std::shared_ptr<PresetModel>> m_PresetsList;
        std::atomic_int m_LastUsedID;
        // timer needs to be here because of the multithreading
        QTimer m_SavingTimer;
    };

    class FilteredPresetsModelBase:
            public QSortFilterProxyModel
    {
        Q_OBJECT
    public:
        FilteredPresetsModelBase(PresetKeywordsModel &presetsModel);

    public:
        Q_INVOKABLE int getOriginalIndex(int index);
        Q_INVOKABLE unsigned int getOriginalID(int index);
        Q_INVOKABLE int getItemsCount() const { return rowCount(); }
        Q_INVOKABLE QString getName(int index);
    protected:
        PresetKeywordsModel &getPresetsModel() const;

    private:
        PresetKeywordsModel &m_PresetKeywordsModel;
    };

    class FilteredPresetKeywordsModel:
        public FilteredPresetsModelBase
    {
        Q_OBJECT
        Q_PROPERTY(QString searchTerm READ getSearchTerm WRITE setSearchTerm NOTIFY searchTermChanged)

    public:
        FilteredPresetKeywordsModel(PresetKeywordsModel &presetsModel);
    public:
        const QString &getSearchTerm() const { return m_SearchTerm; }
        void setSearchTerm(const QString &value);
    signals:
        void searchTermChanged(const QString &searchTerm);
        // QSortFilterProxyModel interface
    protected:
        virtual bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    private:
        QString m_SearchTerm;
    };

    class PresetKeywordsGroupModel:
            public FilteredPresetsModelBase
    {
        Q_OBJECT
    public:
        PresetKeywordsGroupModel(int groupID, PresetKeywordsModel &presetsModel);

    public:
        int getGroupID() const { return m_GroupID; }

        // QSortFilterProxyModel interface
    protected:
        virtual bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    private:
        int m_GroupID;
    };
}
#endif // PRESETKEYWORDSMODEL_H

/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2017 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef AUTOCOMPLETEMODEL_H
#define AUTOCOMPLETEMODEL_H

#include <QAbstractListModel>
#include <QVector>
#include <QString>
#include "../Common/baseentity.h"

namespace AutoComplete {
    class AutoCompleteModel : public QAbstractListModel, public Common::BaseEntity
    {
        Q_OBJECT
        Q_PROPERTY(int selectedIndex READ getSelectedIndex WRITE setSelectedIndex NOTIFY selectedIndexChanged)
        Q_PROPERTY(bool isActive READ getIsActive WRITE setIsActive NOTIFY isActiveChanged)
    public:
        AutoCompleteModel(QObject *parent=0);

    public:
        enum AutoCompleteModelRoles {
            IsPresetRole = Qt::UserRole + 1,
        };

    public:
        void setCompletions(const QStringList &completions);

    public:
        int getSelectedIndex() const { return m_SelectedIndex; }
        bool getIsActive() const { return m_IsActive; }

    public:
        void setSelectedIndex(int value) {
            if (value != m_SelectedIndex) {
                m_SelectedIndex = value;
                emit selectedIndexChanged(value);
            }
        }

        void setIsActive(bool value) {
            if (value != m_IsActive) {
                m_IsActive = value;
                emit isActiveChanged(value);
            }
        }

    public:
        Q_INVOKABLE bool moveSelectionUp();
        Q_INVOKABLE bool moveSelectionDown();
        Q_INVOKABLE void cancelCompletion() { setIsActive(false); emit dismissPopupRequested(); }
        Q_INVOKABLE void acceptSelected(bool tryExpandPreset=false);
        Q_INVOKABLE int getCount() const { return qMax(m_CompletionList.length(), m_LastGeneratedCompletions.length()); }
        Q_INVOKABLE bool hasSelectedCompletion() const { return (0 <= m_SelectedIndex) && (m_SelectedIndex < m_CompletionList.length()); }

    signals:
        void selectedIndexChanged(int index);
        void dismissPopupRequested();
        void completionAccepted(const QString &completion, bool expandPreset);
        void isActiveChanged(bool value);
#ifdef INTEGRATION_TESTS
        void completionsUpdated();
#endif

    public slots:
        void completionsArrived();

        // QAbstractItemModel interface
    public:
        virtual int rowCount(const QModelIndex &parent) const override { Q_UNUSED(parent); return m_CompletionList.length(); }
        virtual QVariant data(const QModelIndex &index, int role) const override;
        virtual QHash<int, QByteArray> roleNames() const override;

#ifdef INTEGRATION_TESTS
        bool containsWord(const QString &word) { return m_CompletionList.contains(word); }
        const QStringList &getLastGeneratedCompletions() const { return m_LastGeneratedCompletions; }
#endif

    private:
        QVector<QString> m_CompletionList;
        QStringList m_LastGeneratedCompletions;
        int m_SelectedIndex;
        bool m_IsActive;
    };
}

#endif // AUTOCOMPLETEMODEL_H

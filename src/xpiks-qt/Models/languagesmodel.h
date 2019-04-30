/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef LANGUAGESMODEL_H
#define LANGUAGESMODEL_H

#include <QAbstractListModel>
#include <QDir>
#include <QHash>
#include <QModelIndex>
#include <QObject>
#include <QPair>
#include <QString>
#include <QVariant>
#include <QVector>
#include <Qt>
#include <QtGlobal>

class QByteArray;
class QModelIndex;
class QTranslator;

namespace Models {
    class SettingsModel;

    class LanguagesModel: public QAbstractListModel
    {
        Q_OBJECT
        Q_PROPERTY(QString n READ getEmptyString() NOTIFY languageChanged)
    public:
        LanguagesModel(Models::SettingsModel &settingsModel, QObject *parent=nullptr);

    public:
        enum LanguagesModel_Roles {
            IsCurrentRole = Qt::UserRole + 1
        };

        const QString &getEmptyString() const { return m_EmptyString; }

    signals:
        void languageChanged();

    public:
        void initFirstLanguage();
        QString getCurrentLanguage() const;
        Q_INVOKABLE void loadLanguages();
        Q_INVOKABLE void switchLanguage(int index);

    public:
        virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override { Q_UNUSED(parent); return m_LanguagesList.length(); }
        virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    protected:
        virtual QHash<int, QByteArray> roleNames() const override;

    private:
        void loadTranslators(const QDir &dir, const QString &selectedLocale);
        QString getTranslationsPath() const;

    private:
        QVector<QPair<QString, QString> > m_LanguagesList;
        QTranslator *m_XpiksTranslator;
        QTranslator *m_QtTranslator;
        SettingsModel &m_SettingsModel;
        QString m_EmptyString;
        QString m_TranslationsPath;
        int m_CurrentLanguageIndex;
    };
}

#endif // LANGUAGESMODEL_H

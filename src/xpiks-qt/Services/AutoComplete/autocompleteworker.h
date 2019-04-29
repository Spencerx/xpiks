/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef AUTOCOMPLETEWORKER_H
#define AUTOCOMPLETEWORKER_H

#include <memory>

#include <QObject>
#include <QString>

#include "Common/itemprocessingworker.h"
#include "Services/AutoComplete/libfacecompletionengine.h"
#include "Services/AutoComplete/presetscompletionengine.h"

namespace Helpers {
    class AsyncCoordinator;
}

namespace KeywordsPresets {
    class PresetKeywordsModel;
    class IPresetsManager;
}

namespace AutoComplete {
    class CompletionQuery;
    class KeywordsAutoCompleteModel;

    class AutoCompleteWorker :
            public QObject,
            public Common::ItemProcessingWorker<CompletionQuery>
    {
        Q_OBJECT
    public:
        explicit AutoCompleteWorker(Helpers::AsyncCoordinator &initCoordinator,
                                    KeywordsAutoCompleteModel &autoCompleteModel,
                                    KeywordsPresets::PresetKeywordsModel &presetsManager,
                                    QObject *parent = nullptr);
        virtual ~AutoCompleteWorker();

    protected:
        virtual bool initWorker() override;
        virtual void processOneItem(const std::shared_ptr<CompletionQuery> &item) override;

    private:
        void generateCompletions(const std::shared_ptr<CompletionQuery> &item);
        void updateCompletions(const std::shared_ptr<CompletionQuery> &item);

    protected:
        virtual void onQueueIsEmpty() override { emit queueIsEmpty(); }
        virtual void onWorkerStopped() override { emit stopped(); }

    public slots:
        void process() { doWork(); }
        void cancel() { stopWorking(); }

    signals:
        void stopped();
        void queueIsEmpty();

    private:
        LibFaceCompletionEngine m_FaceCompletionEngine;
        PresetsCompletionEngine m_PresetsCompletionEngine;
        Helpers::AsyncCoordinator &m_InitCoordinator;
        KeywordsAutoCompleteModel &m_AutoCompleteModel;
        KeywordsPresets::IPresetsManager &m_PresetsManager;
    };
}

#endif // AUTOCOMPLETEWORKER_H

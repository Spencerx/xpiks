/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "autocompleteworker.h"
#include <QDir>
#include <QStringList>
#include <Common/defines.h>
#include "keywordsautocompletemodel.h"
#include <Helpers/asynccoordinator.h>
#include <KeywordsPresets/ipresetsmanager.h>
#include <KeywordsPresets/presetkeywordsmodel.h>
#include <Artworks/basickeywordsmodel.h>

namespace AutoComplete {
    AutoCompleteWorker::AutoCompleteWorker(Helpers::AsyncCoordinator &initCoordinator,
                                           KeywordsAutoCompleteModel &autoCompleteModel,
                                           KeywordsPresets::PresetKeywordsModel &presetsManager,
                                           QObject *parent) :
        QObject(parent),
        m_PresetsCompletionEngine(presetsManager),
        m_InitCoordinator(initCoordinator),
        m_AutoCompleteModel(autoCompleteModel),
        m_PresetsManager(presetsManager)
    {
    }

    AutoCompleteWorker::~AutoCompleteWorker() {
        m_FaceCompletionEngine.finalize();
        m_PresetsCompletionEngine.finalize();
        LOG_INFO << "destroyed";
    }

    bool AutoCompleteWorker::initWorker() {
        LOG_INFO << "#";

        Helpers::AsyncCoordinatorUnlocker unlocker(m_InitCoordinator);
        Q_UNUSED(unlocker);

        bool success = false;

        do {
            if (!m_FaceCompletionEngine.initialize()) {
                break;
            }

            if (!m_PresetsCompletionEngine.initialize()) {
                break;
            }

            success = true;
        } while(false);

        return success;
    }

    void AutoCompleteWorker::processOneItem(std::shared_ptr<CompletionQuery> &item) {
        if (!item->getNeedsUpdate()) {
            generateCompletions(item);
        } else {
            updateCompletions(item);
        }
    }

    void AutoCompleteWorker::generateCompletions(std::shared_ptr<CompletionQuery> &item) {
        const QString &prefix = item->getPrefix();
        LOG_VERBOSE_OR_DEBUG << prefix;

        bool onlyFindPresets = false;

        if (item->getCompletePresets()) {
            if (item->getCompleteKeywords()) {
                if (prefix.startsWith(PRESETS_COMPLETE_PREFIX)) {
                    onlyFindPresets = true;
                }
            } else {
                onlyFindPresets = true;
            }
        }

        std::vector<CompletionResult> completionsList;
        size_t generatedCount = 0;

        if (onlyFindPresets) {
            if (m_PresetsCompletionEngine.generateCompletions(*item.get(), completionsList)) {
                generatedCount = completionsList.size();
                auto &completionsModel = m_AutoCompleteModel.getCompletionsSource();
                completionsModel.setPresetCompletions(completionsList);
            }
        } else {
            if (m_FaceCompletionEngine.generateCompletions(*item.get(), completionsList)) {
                generatedCount = completionsList.size();
                auto &completionsModel = m_AutoCompleteModel.getCompletionsSource();
                completionsModel.setKeywordCompletions(completionsList);

                item->setCompletions(completionsList);
                item->setNeedsUpdate();

                this->submitFirst(item);
            }
        }

        if (generatedCount > 0) {
            item->notifyCompletionsAvailable();
        }
    }

    void AutoCompleteWorker::updateCompletions(std::shared_ptr<CompletionQuery> &item) {
        LOG_VERBOSE_OR_DEBUG << item->getPrefix();

        bool anyChanges = false;
        auto &completionsList = item->getCompletions();

        LOG_VERBOSE_OR_DEBUG << "Updating" << completionsList.size() << "items";

        for (auto &result: completionsList) {
            KeywordsPresets::ID_t presetID;
            if (m_PresetsManager.tryFindSinglePresetByName(result.m_Completion, false, presetID)) {
                result.m_PresetID = presetID;
                anyChanges = true;
            }
        }

        if (anyChanges) {
            LOG_VERBOSE_OR_DEBUG << "Propagating update to the model";
            auto &completionsModel = m_AutoCompleteModel.getCompletionsSource();
            completionsModel.setPresetsMembership(completionsList);
        }
    }
}

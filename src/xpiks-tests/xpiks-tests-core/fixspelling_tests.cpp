#include "fixspelling_tests.h"
#include <QSignalSpy>
#include "Mocks/commandmanagermock.h"
#include "Mocks/spellcheckservicemock.h"
#include "Mocks/coretestsenvironment.h"
#include "Mocks/flagsprovidermock.h"
#include "Mocks/artworksupdatermock.h"
#include <Common/flags.h>
#include <UndoRedo/undoredomanager.h>
#include <Artworks/basicmetadatamodel.h>
#include <Services/SpellCheck/spellsuggestionsitem.h>
#include <Services/SpellCheck/spellchecksuggestionmodel.h>
#include <Services//SpellCheck/spellcheckiteminfo.h>

#define INIT_FIX_SPELLING_TEST \
    Mocks::CoreTestsEnvironment environment; \
    UndoRedo::UndoRedoManager undoRedoManager;\
    Mocks::CommandManagerMock commandManager(undoRedoManager); \
    Mocks::FlagsProviderMock<Common::WordAnalysisFlags> flagsProvider(Common::WordAnalysisFlags::All);\
    Mocks::SpellCheckServiceMock spellCheckService(environment, flagsProvider); \
    Mocks::ArtworksUpdaterMock artworksUpdater;\
    SpellCheck::SpellCheckSuggestionModel suggestionModel(spellCheckService, artworksUpdater); \
    Artworks::BasicMetadataModel basicModel(m_FakeHold); \
    SpellCheck::SpellCheckItemInfo spellCheckInfo; \
    basicModel.setSpellCheckInfo(&spellCheckInfo);

void FixSpellingTests::fixKeywordsSmokeTest() {
    INIT_FIX_SPELLING_TEST;

    QSignalSpy spellCheckSpy(&basicModel, SIGNAL(keywordsSpellingChanged()));

    basicModel.initialize("title", "description", "keyword1, keyword2");
    basicModel.getRawKeywords()[0].m_IsCorrect = false;
    suggestionModel.setupItem(&basicModel);

    SpellCheck::SpellSuggestionsItem *suggestionItem = suggestionModel.getItem(0);
    QString itemToReplace = "item1";
    suggestionItem->setSuggestions(QStringList() << itemToReplace << "item2");
    suggestionItem->setReplacementIndex(0);

    suggestionModel.submitCorrections();

    QCOMPARE(basicModel.getKeywords()[0], itemToReplace);
    QCOMPARE(spellCheckSpy.count(), 1);
}

void FixSpellingTests::noReplacementsSelectedTest() {
    INIT_FIX_SPELLING_TEST;

    QSignalSpy spellCheckSpy(&basicModel, SIGNAL(keywordsSpellingChanged()));

    basicModel.initialize("title", "description", "keyword1, keyword2");
    basicModel.getRawKeywords()[0].m_IsCorrect = false;

    suggestionModel.setupItem(&basicModel);

    suggestionModel.submitCorrections();

    QCOMPARE(basicModel.getKeywords()[0], QString("keyword1"));
    QVERIFY(spellCheckSpy.isEmpty());
}

void FixSpellingTests::fixAndRemoveDuplicatesTest() {
    INIT_FIX_SPELLING_TEST;

    QSignalSpy spellCheckSpy(&basicModel, SIGNAL(keywordsSpellingChanged()));

    basicModel.initialize("title", "description", "keyword1, keyword2");
    basicModel.getRawKeywords()[0].m_IsCorrect = false;
    basicModel.getRawKeywords()[1].m_IsCorrect = false;

    suggestionModel.setupItem(&basicModel);

    QCOMPARE(basicModel.getKeywordsCount(), 2);

    SpellCheck::SpellSuggestionsItem *suggestionItem = suggestionModel.getItem(0);
    suggestionItem->setReplacementIndex(0);

    SpellCheck::SpellSuggestionsItem *otherSuggestionItem = suggestionModel.getItem(1);
    otherSuggestionItem->setReplacementIndex(0);

    suggestionModel.submitCorrections();

    QCOMPARE(basicModel.getKeywords()[0], QString("item1"));
    QCOMPARE(basicModel.getKeywordsCount(), 1);
    QCOMPARE(spellCheckSpy.count(), 1);
}

void FixSpellingTests::fixAndRemoveDuplicatesCombinedTest() {
    INIT_FIX_SPELLING_TEST;

    QSignalSpy spellCheckSpy(&basicModel, SIGNAL(keywordsSpellingChanged()));

    basicModel.initialize("title", "description", "keyword1, keyword2 item1 test, keyword2 wordtoreplace test");
    basicModel.getRawKeywords()[0].m_IsCorrect = false;
    basicModel.getRawKeywords()[2].m_IsCorrect = false;

    suggestionModel.setupItem(&basicModel); // keywords

    QCOMPARE(basicModel.getKeywordsCount(), 3);
    qDebug() << basicModel.getKeywords();
    QCOMPARE(suggestionModel.rowCount(), 4);

    for (int i = 0; i < suggestionModel.rowCount(); ++i) {
        SpellCheck::SpellSuggestionsItem *suggestionItem = suggestionModel.getItem(i);
        if (suggestionItem->getWord() == "wordtoreplace") {
            suggestionItem->setReplacementIndex(0);
        }
    }

    suggestionModel.submitCorrections();

    QCOMPARE(basicModel.getKeywordsCount(), 2);
    QCOMPARE(spellCheckSpy.count(), 1);
}

void FixSpellingTests::multiReplaceWithCorrectAllTest() {
    INIT_FIX_SPELLING_TEST;

    QSignalSpy spellCheckSpy(&basicModel, SIGNAL(keywordsSpellingChanged()));

    basicModel.initialize("wordtoreplace in title", "description has wordtoreplace too", "wordtoreplace, keyword2, word plus wordtoreplace");
    basicModel.getRawKeywords()[0].m_IsCorrect = false;
    basicModel.getRawKeywords()[2].m_IsCorrect = false;
    spellCheckInfo.setDescriptionErrors(QSet<QString>() << "wordtoreplace");
    spellCheckInfo.setTitleErrors(QSet<QString>() << "wordtoreplace");

    suggestionModel.setupItem(&basicModel);

    QCOMPARE(suggestionModel.rowCount(), 3);
    for (int i = 0; i < suggestionModel.rowCount(); ++i) {
        SpellCheck::SpellSuggestionsItem *suggestionItem = suggestionModel.getItem(i);
        SpellCheck::CombinedSpellSuggestions *combinedItem = dynamic_cast<SpellCheck::CombinedSpellSuggestions*>(suggestionItem);
        if (combinedItem != NULL) {
            suggestionItem->setReplacementIndex(0);
        }
    }

    suggestionModel.submitCorrections();

    QCOMPARE(basicModel.getKeywords()[0], QString("item1"));
    QCOMPARE(basicModel.getKeywords()[2], QString("word plus item1"));
    QCOMPARE(basicModel.getDescription(), QString("description has item1 too"));
    QCOMPARE(basicModel.getTitle(), QString("item1 in title"));
    QCOMPARE(spellCheckSpy.count(), 1);
}

void FixSpellingTests::replaceWithCorrectDescriptionTest() {
    INIT_FIX_SPELLING_TEST;

    QSignalSpy spellCheckSpy(&basicModel, SIGNAL(keywordsSpellingChanged()));

    basicModel.initialize("wordtoreplace in title", "description has wordtoreplace too", "wordtoreplace, keyword2, word plus wordtoreplace");
    basicModel.getRawKeywords()[0].m_IsCorrect = false;
    basicModel.getRawKeywords()[2].m_IsCorrect = false;
    spellCheckInfo.setDescriptionErrors(QSet<QString>() << "wordtoreplace");
    spellCheckInfo.setTitleErrors(QSet<QString>() << "wordtoreplace");

    suggestionModel.setupItem(&basicModel); // description

    QCOMPARE(suggestionModel.rowCount(), 1);
    SpellCheck::SpellSuggestionsItem *suggestionItem = suggestionModel.getItem(0);
    suggestionItem->setReplacementIndex(0);

    suggestionModel.submitCorrections();

    QCOMPARE(basicModel.getKeywords()[0], QString("wordtoreplace"));
    QCOMPARE(basicModel.getKeywords()[2], QString("word plus wordtoreplace"));
    QCOMPARE(basicModel.getDescription(), QString("description has item1 too"));
    QCOMPARE(basicModel.getTitle(), QString("wordtoreplace in title"));
    QCOMPARE(spellCheckSpy.count(), 1);
}

void FixSpellingTests::replaceWithCorrectTitleTest() {
    INIT_FIX_SPELLING_TEST;

    QSignalSpy spellCheckSpy(&basicModel, SIGNAL(keywordsSpellingChanged()));

    basicModel.initialize("wordtoreplace in title", "description has wordtoreplace too", "wordtoreplace, keyword2, word plus wordtoreplace");
    basicModel.getRawKeywords()[0].m_IsCorrect = false;
    basicModel.getRawKeywords()[2].m_IsCorrect = false;
    spellCheckInfo.setDescriptionErrors(QSet<QString>() << "wordtoreplace");
    spellCheckInfo.setTitleErrors(QSet<QString>() << "wordtoreplace");

    suggestionModel.setupItem(&basicModel); // title

    QCOMPARE(suggestionModel.rowCount(), 1);
    SpellCheck::SpellSuggestionsItem *suggestionItem = suggestionModel.getItem(0);
    suggestionItem->setReplacementIndex(0);

    suggestionModel.submitCorrections();

    QCOMPARE(basicModel.getKeywords()[0], QString("wordtoreplace"));
    QCOMPARE(basicModel.getKeywords()[2], QString("word plus wordtoreplace"));
    QCOMPARE(basicModel.getDescription(), QString("description has wordtoreplace too"));
    QCOMPARE(basicModel.getTitle(), QString("item1 in title"));
    QCOMPARE(spellCheckSpy.count(), 1);
}

void FixSpellingTests::replaceWithCorrectKeywordsTest() {
    INIT_FIX_SPELLING_TEST;

    QSignalSpy spellCheckSpy(&basicModel, SIGNAL(keywordsSpellingChanged()));

    basicModel.initialize("wordtoreplace in title", "description has wordtoreplace too", "wordtoreplace, keyword2, word plus wordtoreplace");
    basicModel.getRawKeywords()[0].m_IsCorrect = false;
    basicModel.getRawKeywords()[2].m_IsCorrect = false;
    spellCheckInfo.setDescriptionErrors(QSet<QString>() << "wordtoreplace");
    spellCheckInfo.setTitleErrors(QSet<QString>() << "wordtoreplace");

    suggestionModel.setupItem(&basicModel); // keywords

    QCOMPARE(suggestionModel.rowCount(), 3);
    for (int i = 0; i < suggestionModel.rowCount(); ++i) {
        SpellCheck::SpellSuggestionsItem *suggestionItem = suggestionModel.getItem(i);
        SpellCheck::CombinedSpellSuggestions *combinedItem = dynamic_cast<SpellCheck::CombinedSpellSuggestions*>(suggestionItem);
        if (combinedItem != NULL) {
            suggestionItem->setReplacementIndex(0);
        }
    }

    suggestionModel.submitCorrections();

    QCOMPARE(basicModel.getKeywords()[0], QString("item1"));
    QCOMPARE(basicModel.getKeywords()[2], QString("word plus item1"));
    QCOMPARE(basicModel.getDescription(), QString("description has wordtoreplace too"));
    QCOMPARE(basicModel.getTitle(), QString("wordtoreplace in title"));
    QCOMPARE(spellCheckSpy.count(), 1);
}

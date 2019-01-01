#include "keywordssuggestion_tests.h"    

#include <QSet>
#include <QString>

#include <Suggestion/suggestedkeywords.h>

void KeywordsSuggestionTests::suggestSingleKeywordsSetTest() {
    Suggestion::SuggestedKeywords suggestedKeywords;
    QSet<QString> keywords = { "random", "test", "keywords", "fake",
                               "forest", "linux"};

    suggestedKeywords.addKeywords(keywords);
    suggestedKeywords.updateSuggestion(1);

    QCOMPARE(suggestedKeywords.getSuggestedKeywords().length(), keywords.size());
    QCOMPARE(suggestedKeywords.getSuggestedKeywords().toSet(),
             keywords);
}

void KeywordsSuggestionTests::suggestKeywordsSimpleTest() {
    Suggestion::SuggestedKeywords suggestedKeywords;

    suggestedKeywords.addKeywords(QSet<QString>({"graphic", "vector", "illustration"}));
    suggestedKeywords.addKeywords(QSet<QString>({"graphic", "person", "line", "vector"}));
    suggestedKeywords.addKeywords(QSet<QString>({"art", "people", "line", "vector"}));

    suggestedKeywords.updateSuggestion(3);

    QCOMPARE(suggestedKeywords.getSuggestedKeywords().length(), 3);
    QCOMPARE(suggestedKeywords.getOtherKeywords().length(), 4);
    QCOMPARE(suggestedKeywords.getSuggestedKeywords().toSet(),
             QSet<QString>({"graphic", "line", "vector"}));
    QCOMPARE(suggestedKeywords.getOtherKeywords().toSet(),
             QSet<QString>({ "illustration", "person", "art", "people" }));
}

void KeywordsSuggestionTests::repeatableSuggestionTest() {
    Suggestion::SuggestedKeywords suggestedKeywords;

    int n = 100;

    while (n--) {
        suggestedKeywords.addKeywords(QSet<QString>({"graphic", "vector", "illustration"}));
        suggestedKeywords.addKeywords(QSet<QString>({"graphic", "person", "line", "vector"}));
        suggestedKeywords.addKeywords(QSet<QString>({"art", "people", "line", "vector"}));

        suggestedKeywords.updateSuggestion(3);

        QCOMPARE(suggestedKeywords.getSuggestedKeywords().length(), 3);
        QCOMPARE(suggestedKeywords.getSuggestedKeywords().toSet(),
                 QSet<QString>({"graphic", "line", "vector"}));

        suggestedKeywords.reset();
    }
}

void KeywordsSuggestionTests::multipleUpdateSuggestionTest() {
    Suggestion::SuggestedKeywords suggestedKeywords;

    suggestedKeywords.addKeywords(QSet<QString>({"graphic", "vector", "illustration"}));
    suggestedKeywords.addKeywords(QSet<QString>({"graphic", "person", "line", "vector"}));
    suggestedKeywords.addKeywords(QSet<QString>({"art", "people", "line", "vector"}));

    int n = 10;

    while (n--) {
        suggestedKeywords.updateSuggestion(3);
        QCOMPARE(suggestedKeywords.getSuggestedKeywords().length(), 3);
        QCOMPARE(suggestedKeywords.getSuggestedKeywords().toSet(),
                 QSet<QString>({"graphic", "line", "vector"}));
    }
}

void KeywordsSuggestionTests::excludesExistingKeywordsForOneTest() {
    Suggestion::SuggestedKeywords suggestedKeywords;
    QSet<QString> keywords = { "random", "test", "keywords", "fake",
                               "forest", "linux"};

    suggestedKeywords.setExistingKeywords(QSet<QString>({ "random", "test", "keywords", "fake" }));
    suggestedKeywords.addKeywords(keywords);
    suggestedKeywords.updateSuggestion(1);

    QCOMPARE(suggestedKeywords.getSuggestedKeywords().length(), 2);
    QCOMPARE(suggestedKeywords.getSuggestedKeywords().toSet(),
             QSet<QString>({ "forest", "linux" }));
}

void KeywordsSuggestionTests::excludesExistingKeywordsTest() {
    Suggestion::SuggestedKeywords suggestedKeywords;

    suggestedKeywords.addKeywords(QSet<QString>({"graphic", "vector", "illustration"}));
    suggestedKeywords.addKeywords(QSet<QString>({"graphic", "person", "line", "vector"}));
    suggestedKeywords.addKeywords(QSet<QString>({"art", "people", "line", "vector"}));

    suggestedKeywords.setExistingKeywords(QSet<QString>({ "graphic", "line" }));
    suggestedKeywords.updateSuggestion(3);

    QCOMPARE(suggestedKeywords.getSuggestedKeywords().length(), 1);
    QCOMPARE(suggestedKeywords.getOtherKeywords().length(), 4);
    QCOMPARE(suggestedKeywords.getSuggestedKeywords().toSet(),
             QSet<QString>({ "vector" }));
    QCOMPARE(suggestedKeywords.getOtherKeywords().toSet(),
             QSet<QString>({ "illustration", "person", "art", "people" }));
}

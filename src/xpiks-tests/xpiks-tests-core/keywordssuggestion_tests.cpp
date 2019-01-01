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
    QCOMPARE(suggestedKeywords.getSuggestedKeywords().toSet(),
             keywords);
}

void KeywordsSuggestionTests::suggestKeywordsSimpleTest() {
    Suggestion::SuggestedKeywords suggestedKeywords;

    suggestedKeywords.addKeywords(QSet<QString>({"graphic", "vector", "illustration"}));
    suggestedKeywords.addKeywords(QSet<QString>({"graphic", "person", "line", "vector"}));
    suggestedKeywords.addKeywords(QSet<QString>({"art", "people", "line", "vector"}));

    suggestedKeywords.updateSuggestion(3);

    QCOMPARE(suggestedKeywords.getSuggestedKeywords().toSet(),
             QSet<QString>({"graphic", "line", "vector"}));
}

void KeywordsSuggestionTests::repeatableSuggestionTest() {
    Suggestion::SuggestedKeywords suggestedKeywords;

    int n = 1000;

    while (n--) {
        suggestedKeywords.addKeywords(QSet<QString>({"graphic", "vector", "illustration"}));
        suggestedKeywords.addKeywords(QSet<QString>({"graphic", "person", "line", "vector"}));
        suggestedKeywords.addKeywords(QSet<QString>({"art", "people", "line", "vector"}));

        suggestedKeywords.updateSuggestion(3);

        QCOMPARE(suggestedKeywords.getSuggestedKeywords().toSet(),
                 QSet<QString>({"graphic", "line", "vector"}));

        suggestedKeywords.reset();
    }
}

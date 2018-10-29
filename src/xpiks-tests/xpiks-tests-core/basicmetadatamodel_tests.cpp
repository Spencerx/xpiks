#include "basicmetadatamodel_tests.h"

#include <QLatin1String>
#include <QList>
#include <QSet>
#include <QSignalSpy>
#include <QStringList>
#include <QVariant>

#include "Artworks/basicmetadatamodel.h"
#include "Common/flags.h"

void BasicKeywordsModelTests::constructEmptyTest() {
    Artworks::BasicMetadataModel basicModel(m_SpellCheckInfo);

    QCOMPARE(basicModel.isDescriptionEmpty(), true);
    QCOMPARE(basicModel.isTitleEmpty(), true);
    QCOMPARE(basicModel.isEmpty(), true);
    QCOMPARE(basicModel.getKeywordsCount(), 0);
}

void BasicKeywordsModelTests::simpleAddKeywordTest() {
    Artworks::BasicMetadataModel basicModel(m_SpellCheckInfo);

    QSignalSpy addSignalSpy(&basicModel, SIGNAL(rowsInserted(QModelIndex,int,int)));

    bool result = basicModel.appendKeyword("test keyword");
    QCOMPARE(result, true);

    QCOMPARE(addSignalSpy.count(), 1);
    QCOMPARE(basicModel.getKeywordsCount(), 1);
    QList<QVariant> addedArguments = addSignalSpy.takeFirst();
    QCOMPARE(addedArguments.at(1).toInt(), 0);
    QCOMPARE(addedArguments.at(2).toInt(), 0);
}

void BasicKeywordsModelTests::simpleSetKeywordsTest() {
    Artworks::BasicMetadataModel basicModel(m_SpellCheckInfo);

    basicModel.setKeywords(QStringList() << "downhill" << "slope" << "uphill" << "slope");
    QCOMPARE(basicModel.getKeywordsCount(), 3);
    QStringList expectedKeywords;
    expectedKeywords << "downhill" << "slope" << "uphill";
    QCOMPARE(basicModel.getKeywords(), expectedKeywords);
}

void BasicKeywordsModelTests::addExistingKeywordTest() {
    Artworks::BasicMetadataModel basicModel(m_SpellCheckInfo);

    bool result = basicModel.appendKeyword("test keyword");
    QCOMPARE(result, true);

    QSignalSpy addSignalSpy(&basicModel, SIGNAL(rowsInserted(QModelIndex,int,int)));

    result = basicModel.appendKeyword("test keyword");
    QCOMPARE(result, false);

    QCOMPARE(addSignalSpy.count(), 0);
    QCOMPARE(basicModel.getKeywordsCount(), 1);
}

void BasicKeywordsModelTests::addSeveralKeywordsTest() {
    Artworks::BasicMetadataModel basicModel(m_SpellCheckInfo);

    QSignalSpy addSignalSpy(&basicModel, SIGNAL(rowsInserted(QModelIndex,int,int)));

    QStringList keywords;
    keywords << "keyword1" << "keyword2" << "keyword3";

    int appendedCount = basicModel.appendKeywords(keywords);
    QCOMPARE(appendedCount, keywords.length());

    QCOMPARE(addSignalSpy.count(), 1);
    QCOMPARE(basicModel.getKeywordsCount(), keywords.length());
    QList<QVariant> addedArguments = addSignalSpy.takeFirst();
    QCOMPARE(addedArguments.at(1).toInt(), 0);
    QCOMPARE(addedArguments.at(2).toInt(), keywords.length() - 1);
}

void BasicKeywordsModelTests::removeExistingKeywordTest() {
    Artworks::BasicMetadataModel basicModel(m_SpellCheckInfo);

    QString keyword = "test keyword";
    int appendedCount = basicModel.appendKeyword(keyword);
    QCOMPARE(appendedCount, 1);

    QSignalSpy removeSignalSpy(&basicModel, SIGNAL(rowsRemoved(QModelIndex,int,int)));

    QString removedKeyword;
    bool result = basicModel.removeKeywordAt(0, removedKeyword);
    QCOMPARE(result, true);

    QCOMPARE(removedKeyword, keyword);

    QCOMPARE(basicModel.getKeywordsCount(), 0);
    QCOMPARE(removeSignalSpy.count(), 1);
    QList<QVariant> removedArguments = removeSignalSpy.takeFirst();
    QCOMPARE(removedArguments.at(1).toInt(), 0);
    QCOMPARE(removedArguments.at(2).toInt(), 0);
}

void BasicKeywordsModelTests::removeNonExistingKeywordTest() {
    Artworks::BasicMetadataModel basicModel(m_SpellCheckInfo);

    bool result = basicModel.appendKeyword("test keyword");
    QCOMPARE(result, true);

    QSignalSpy removeSignalSpy(&basicModel, SIGNAL(rowsRemoved(QModelIndex,int,int)));

    QString removedKeyword;
    result = basicModel.removeKeywordAt(-1, removedKeyword);
    QCOMPARE(result, false);
    QCOMPARE(basicModel.getKeywordsCount(), 1);
    QCOMPARE(removeSignalSpy.count(), 0);

    result = basicModel.removeKeywordAt(1, removedKeyword);
    QCOMPARE(result, false);
    QCOMPARE(basicModel.getKeywordsCount(), 1);
    QCOMPARE(removeSignalSpy.count(), 0);
}

void BasicKeywordsModelTests::appendSameKeywordsTest() {
    Artworks::BasicMetadataModel basicModel(m_SpellCheckInfo);

    QSignalSpy addSignalSpy(&basicModel, SIGNAL(rowsInserted(QModelIndex,int,int)));

    QString keyword = "test keyword";
    QStringList keywords;
    keywords << keyword << keyword << keyword << keyword;

    int appendedCount = basicModel.appendKeywords(keywords);
    QCOMPARE(appendedCount, 1);

    QCOMPARE(addSignalSpy.count(), 1);
    QCOMPARE(basicModel.getKeywordsCount(), 1);
    QList<QVariant> addedArguments = addSignalSpy.takeFirst();
    QCOMPARE(addedArguments.at(1).toInt(), 0);
    QCOMPARE(addedArguments.at(2).toInt(), 0);
}

void BasicKeywordsModelTests::appendSameChangedKeywordTest() {
    Artworks::BasicMetadataModel basicModel(m_SpellCheckInfo);

    QSignalSpy addSignalSpy(&basicModel, SIGNAL(rowsInserted(QModelIndex,int,int)));

    QStringList keywords;
    keywords << "keyword" << "KEYWORD" << "kEyworD" << "keYword" << "  keyword" << "keyword  " << "   kEyworD  ";

    int appendedCount = basicModel.appendKeywords(keywords);
    QCOMPARE(appendedCount, 1);

    QCOMPARE(addSignalSpy.count(), 1);
    QCOMPARE(basicModel.getKeywordsCount(), 1);
    QList<QVariant> addedArguments = addSignalSpy.takeFirst();
    QCOMPARE(addedArguments.at(1).toInt(), 0);
    QCOMPARE(addedArguments.at(2).toInt(), 0);
}

void BasicKeywordsModelTests::appendNoKeywordsTest() {
    Artworks::BasicMetadataModel basicModel(m_SpellCheckInfo);

    QSignalSpy addSignalSpy(&basicModel, SIGNAL(rowsInserted(QModelIndex,int,int)));

    QStringList keywords;

    int appendedCount = basicModel.appendKeywords(keywords);
    QCOMPARE(appendedCount, 0);

    QCOMPARE(addSignalSpy.count(), 0);
    QCOMPARE(basicModel.getKeywordsCount(), 0);
}

void BasicKeywordsModelTests::expandPresetTest() {
    Artworks::BasicMetadataModel basicModel(m_SpellCheckInfo);
    basicModel.appendKeywords(QStringList() << "hills" << "mountains" << "away");
    const int initialSize = basicModel.getKeywordsCount();

    QSignalSpy addSignalSpy(&basicModel, SIGNAL(rowsInserted(QModelIndex,int,int)));

    QStringList presetKeywords;
    presetKeywords << "mountains" << "peaks" << "high" << "altitude" << "terrain";

    const int indexToExpand = 1;

    bool expanded = basicModel.expandPreset(indexToExpand, presetKeywords);
    QCOMPARE(expanded, true);

    QCOMPARE(addSignalSpy.count(), 1);
    QCOMPARE(basicModel.getKeywordsCount(), initialSize - 1 + presetKeywords.size());
    QList<QVariant> addedArguments = addSignalSpy.takeFirst();
    QCOMPARE(addedArguments.at(1).toInt(), initialSize - 1);
    QCOMPARE(addedArguments.at(2).toInt(), initialSize - 1 + presetKeywords.size() - 1);
}

void BasicKeywordsModelTests::clearKeywordsTest() {
    Artworks::BasicMetadataModel basicModel(m_SpellCheckInfo);

    QStringList keywords;
    keywords << "keyword1" << "keyword2" << "keyword3";

    QString description = "Some description";
    QString title = "Some title";
    basicModel.setDescription(description);
    basicModel.setTitle(title);
    basicModel.appendKeywords(keywords);

    QSignalSpy clearSignalSpy(&basicModel, SIGNAL(modelReset()));

    basicModel.clearKeywords();

    QCOMPARE(clearSignalSpy.count(), 1);
    QCOMPARE(basicModel.getKeywordsCount(), 0);
    QCOMPARE(basicModel.getTitle(), title);
    QCOMPARE(basicModel.getDescription(), description);
}

void BasicKeywordsModelTests::clearModelTest() {
    Artworks::BasicMetadataModel basicModel(m_SpellCheckInfo);

    QStringList keywords;
    keywords << "keyword1" << "keyword2" << "keyword3";

    QString description = "Some description";
    QString title = "Some title";
    basicModel.setDescription(description);
    basicModel.setTitle(title);
    basicModel.appendKeywords(keywords);

    QSignalSpy clearSignalSpy(&basicModel, SIGNAL(modelReset()));

    basicModel.clearModel();

    QCOMPARE(clearSignalSpy.count(), 1);
    QCOMPARE(basicModel.getKeywordsCount(), 0);
    QVERIFY(basicModel.isDescriptionEmpty());
    QVERIFY(basicModel.isTitleEmpty());
}

void BasicKeywordsModelTests::containsKeywordTest() {
    Artworks::BasicMetadataModel basicModel(m_SpellCheckInfo);

    QStringList keywords;
    keywords << "keyword1" << "keyword2" << "keyword3";
    basicModel.appendKeywords(keywords);

    QVERIFY(basicModel.containsKeyword("keyword1"));
}

void BasicKeywordsModelTests::containsKeywordStrictTest() {
    Artworks::BasicMetadataModel basicModel(m_SpellCheckInfo);

    QStringList keywords;
    keywords << "something_keyword1" << "keyword2" << "keyword3";
    basicModel.appendKeywords(keywords);

    Common::SearchFlags flags = Common::SearchFlags::WholeWords;
    QVERIFY(basicModel.containsKeyword("keyword1", flags) == false);
    QVERIFY(basicModel.containsKeyword("keyword2", flags) == true);
    QVERIFY(basicModel.containsKeyword("kEyworD3", flags) == true);
    Common::ApplyFlag(flags, true, Common::SearchFlags::CaseSensitive);
    QVERIFY(basicModel.containsKeyword("kEyworD3", flags) == false);
}

void BasicKeywordsModelTests::containsKeywordFuzzyTest() {
    Artworks::BasicMetadataModel basicModel(m_SpellCheckInfo);

    QStringList keywords;
    keywords << "something_keyword1" << "keyword2" << "KeyworD3";
    basicModel.appendKeywords(keywords);

    Common::SearchFlags notFuzzyMatchFlags = Common::SearchFlags::Keywords;
    QVERIFY(basicModel.containsKeyword("keyword1", notFuzzyMatchFlags) == true);
    QVERIFY(basicModel.containsKeyword("keyword2", notFuzzyMatchFlags) == true);
    QVERIFY(basicModel.containsKeyword("keyword3", notFuzzyMatchFlags) == true);
}

void BasicKeywordsModelTests::doesNotContainKeywordTest() {
    Artworks::BasicMetadataModel basicModel(m_SpellCheckInfo);

    QStringList keywords;
    keywords << "keyword1" << "keyword2" << "keyword3";
    basicModel.appendKeywords(keywords);

    auto notFuzzyMatchFlags = Common::SearchFlags::ExactKeywords;
    auto fuzzyMatchFlags = Common::SearchFlags::Keywords;
    QVERIFY(basicModel.containsKeyword("keyword4", fuzzyMatchFlags) == false);
    QVERIFY(basicModel.containsKeyword("keyword4", notFuzzyMatchFlags) == false);
    QVERIFY(basicModel.containsKeyword("keyword11", fuzzyMatchFlags) == false);
    QVERIFY(basicModel.containsKeyword("keyword11", notFuzzyMatchFlags) == false);
}

void BasicKeywordsModelTests::setSameTitleTest() {
    Artworks::BasicMetadataModel basicModel(m_SpellCheckInfo);
    bool result = basicModel.setTitle("new title");
    QCOMPARE(result, true);
    result = basicModel.setTitle("new title");
    QCOMPARE(result, false);
}

void BasicKeywordsModelTests::setSameDescriptionTest() {
    Artworks::BasicMetadataModel basicModel(m_SpellCheckInfo);
    bool result = basicModel.setDescription("new description");
    QCOMPARE(result, true);
    result = basicModel.setDescription("new description");
    QCOMPARE(result, false);
}

void BasicKeywordsModelTests::editKeywordToAnotherTest() {
    Artworks::BasicMetadataModel basicModel(m_SpellCheckInfo);

    QStringList keywords;
    keywords << "keyword1" << "keyword2" << "keyword3";
    basicModel.appendKeywords(keywords);

    QSignalSpy modifiedSpy(&basicModel, SIGNAL(dataChanged(QModelIndex, QModelIndex,QVector<int>)));

    QVERIFY(basicModel.canEditKeyword(0, "keyword4"));
    bool editResult = basicModel.editKeyword(0, "keyword4");
    QCOMPARE(editResult, true);
    QCOMPARE(modifiedSpy.count(), 1);
}

void BasicKeywordsModelTests::editKeywordToSameTest() {
    Artworks::BasicMetadataModel basicModel(m_SpellCheckInfo);

    QStringList keywords;
    keywords << "keyword1" << "keyword2" << "keyword3";
    basicModel.appendKeywords(keywords);

    QSignalSpy modifiedSpy(&basicModel, SIGNAL(dataChanged(QModelIndex, QModelIndex,QVector<int>)));

    QVERIFY(!basicModel.canEditKeyword(0, "keyword3"));
    bool editResult = basicModel.editKeyword(0, "keyword3");
    QCOMPARE(editResult, false);
    QCOMPARE(modifiedSpy.count(), 0);
}

void BasicKeywordsModelTests::editKeywordAnotherCaseTest() {
    Artworks::BasicMetadataModel basicModel(m_SpellCheckInfo);

    QStringList keywords;
    keywords << "Mountain";
    basicModel.appendKeywords(keywords);

    QSignalSpy modifiedSpy(&basicModel, SIGNAL(dataChanged(QModelIndex, QModelIndex,QVector<int>)));

    bool editResult = basicModel.editKeyword(0, "mountain");
    QCOMPARE(editResult, true);
    QCOMPARE(basicModel.getKeywordAt(0), QLatin1String("mountain"));
    QCOMPARE(modifiedSpy.count(), 1);
}

void BasicKeywordsModelTests::addRemoveAddUpperCaseWordTest() {
    const QString keyword = "Test";

    Artworks::BasicMetadataModel basicModel(m_SpellCheckInfo);

    bool addResult = basicModel.appendKeyword(keyword);
    QCOMPARE(addResult, true);

    QString removed;
    bool removeResult = basicModel.removeKeywordAt(0, removed);
    QCOMPARE(removeResult, true);
    QCOMPARE(removed, keyword);

    bool addAgainResult = basicModel.appendKeyword(keyword);
    QCOMPARE(addAgainResult, true);
}

void BasicKeywordsModelTests::editToUpperCaseTest() {
    Artworks::BasicMetadataModel basicModel(m_SpellCheckInfo);

    QStringList keywords;
    keywords << "keyword1" << "keyword2" << "keyword3";
    basicModel.appendKeywords(keywords);

    QVERIFY(basicModel.canEditKeyword(0, "Keyword1"));
    bool editSelfResult = basicModel.editKeyword(0, "Keyword1");
    QCOMPARE(editSelfResult, true);

    QVERIFY(!basicModel.canEditKeyword(1, "Keyword1"));
    bool editOtherResult = basicModel.editKeyword(1, "Keyword1");
    QCOMPARE(editOtherResult, false);
}

void BasicKeywordsModelTests::hasKeywordTest() {
    Artworks::BasicMetadataModel basicModel(m_SpellCheckInfo);

    QStringList keywords;
    keywords << "keyword1" << "keyword2" << "keyword3";
    basicModel.appendKeywords(keywords);

    QVERIFY(basicModel.hasKeyword("keyword1"));
    QVERIFY(basicModel.hasKeyword("keYwoRd1  "));
    QVERIFY(basicModel.hasKeyword("   kEyword1"));
    QVERIFY(basicModel.hasKeyword("KEYWORD1"));

    QVERIFY(!basicModel.hasKeyword("keyword1+"));
    QVERIFY(!basicModel.hasKeyword("keyword4+"));
}

void BasicKeywordsModelTests::simpleReplaceTest() {
    Artworks::BasicMetadataModel basicModel(m_SpellCheckInfo);

    QString originalTitle = "Test title here";
    QString originalDescription = "Test description";
    QStringList originalKeywords;
    originalKeywords << "Test keyword1" << "keywTestord2" << "keyword3 Test";

    basicModel.setTitle(originalTitle);
    basicModel.setDescription(originalDescription);
    basicModel.appendKeywords(originalKeywords);

    Common::SearchFlags flags = Common::SearchFlags::Metadata;

    bool replaceSucceeded = basicModel.replace("Test", "Replaced", flags);
    QVERIFY(replaceSucceeded);
    QCOMPARE(basicModel.getTitle(), QLatin1String("Replaced title here"));
    QCOMPARE(basicModel.getDescription(), QLatin1String("Replaced description"));
    QCOMPARE(basicModel.getKeywordAt(0), QLatin1String("Replaced keyword1"));
    QCOMPARE(basicModel.getKeywordAt(1), QLatin1String("keywReplacedord2"));
    QCOMPARE(basicModel.getKeywordAt(2), QLatin1String("keyword3 Replaced"));
}

void BasicKeywordsModelTests::descriptionReplaceTest() {
    Artworks::BasicMetadataModel basicModel(m_SpellCheckInfo);

    QString originalTitle = "Test title here";
    QString originalDescription = "Test description";
    QStringList originalKeywords;
    originalKeywords << "Test keyword1" << "keywTestord2" << "keyword3 Test";

    basicModel.setTitle(originalTitle);
    basicModel.setDescription(originalDescription);
    basicModel.appendKeywords(originalKeywords);

    Common::SearchFlags flags = Common::SearchFlags::Description;

    bool replaceSucceeded = basicModel.replace("Test", "Replaced", flags);
    QVERIFY(replaceSucceeded);
    QCOMPARE(basicModel.getTitle(), originalTitle);
    QCOMPARE(basicModel.getDescription(), QLatin1String("Replaced description"));
    QCOMPARE(basicModel.getKeywordAt(0), originalKeywords[0]);
    QCOMPARE(basicModel.getKeywordAt(1), originalKeywords[1]);
    QCOMPARE(basicModel.getKeywordAt(2), originalKeywords[2]);
}

void BasicKeywordsModelTests::titleReplaceTest() {
    Artworks::BasicMetadataModel basicModel(m_SpellCheckInfo);

    QString originalTitle = "Test title here";
    QString originalDescription = "Test description";
    QStringList originalKeywords;
    originalKeywords << "Test keyword1" << "keywTestord2" << "keyword3 Test";

    basicModel.setTitle(originalTitle);
    basicModel.setDescription(originalDescription);
    basicModel.appendKeywords(originalKeywords);

    Common::SearchFlags flags = Common::SearchFlags::Title;

    bool replaceSucceeded = basicModel.replace("Test", "Replaced", flags);
    QVERIFY(replaceSucceeded);
    QCOMPARE(basicModel.getTitle(), QLatin1String("Replaced title here"));
    QCOMPARE(basicModel.getDescription(), originalDescription);
    QCOMPARE(basicModel.getKeywordAt(0), originalKeywords[0]);
    QCOMPARE(basicModel.getKeywordAt(1), originalKeywords[1]);
    QCOMPARE(basicModel.getKeywordAt(2), originalKeywords[2]);
}

void BasicKeywordsModelTests::keywordsReplaceTest() {
    Artworks::BasicMetadataModel basicModel(m_SpellCheckInfo);

    QString originalTitle = "Test title here";
    QString originalDescription = "Test description";
    QStringList originalKeywords;
    originalKeywords << "Test keyword1" << "keywTestord2" << "keyword3 Test";

    basicModel.setTitle(originalTitle);
    basicModel.setDescription(originalDescription);
    basicModel.appendKeywords(originalKeywords);

    Common::SearchFlags flags = Common::SearchFlags::Keywords;

    bool replaceSucceeded = basicModel.replace("Test", "Replaced", flags);
    QVERIFY(replaceSucceeded);
    QCOMPARE(basicModel.getTitle(), originalTitle);
    QCOMPARE(basicModel.getDescription(), originalDescription);
    QCOMPARE(basicModel.getKeywordAt(0), QLatin1String("Replaced keyword1"));
    QCOMPARE(basicModel.getKeywordAt(1), QLatin1String("keywReplacedord2"));
    QCOMPARE(basicModel.getKeywordAt(2), QLatin1String("keyword3 Replaced"));
}

void BasicKeywordsModelTests::noReplaceCaseSensitiveTest() {
    Artworks::BasicMetadataModel basicModel(m_SpellCheckInfo);

    QString originalTitle = "Test title here";
    QString originalDescription = "Test description";
    QStringList originalKeywords;
    originalKeywords << "Test keyword1" << "keywTestord2" << "keyword3 Test";

    basicModel.setTitle(originalTitle);
    basicModel.setDescription(originalDescription);
    basicModel.appendKeywords(originalKeywords);

    Common::SearchFlags flags = Common::SearchFlags::MetadataCaseSensitive;

    bool replaceSucceeded = basicModel.replace("test", "Replaced", flags);
    QVERIFY(!replaceSucceeded);
}

void BasicKeywordsModelTests::replaceCaseSensitiveTest() {
    Artworks::BasicMetadataModel basicModel(m_SpellCheckInfo);

    QString originalTitle = "Test title test here";
    QString originalDescription = "Test description test";
    QStringList originalKeywords;
    originalKeywords << "Test keyword1" << "keywtestord2" << "keyTestword3 test";

    basicModel.setTitle(originalTitle);
    basicModel.setDescription(originalDescription);
    basicModel.appendKeywords(originalKeywords);

    auto flags = Common::SearchFlags::MetadataCaseSensitive;

    bool replaceSucceeded = basicModel.replace("test", "Replaced", flags);
    QVERIFY(replaceSucceeded);
    QCOMPARE(basicModel.getTitle(), QLatin1String("Test title Replaced here"));
    QCOMPARE(basicModel.getDescription(), QLatin1String("Test description Replaced"));
    QCOMPARE(basicModel.getKeywordAt(0), originalKeywords[0]);
    QCOMPARE(basicModel.getKeywordAt(1), QLatin1String("keywReplacedord2"));
    QCOMPARE(basicModel.getKeywordAt(2), QLatin1String("keyTestword3 Replaced"));
}

void BasicKeywordsModelTests::replaceWholeWordsTest() {
    Artworks::BasicMetadataModel basicModel(m_SpellCheckInfo);

    QString originalTitle = "Test title test here";
    QString originalDescription = "Testdescription test";
    const QStringList originalKeywords(QStringList() << "Test keyword1" << "keywTestord2" << "keyTestword3 test");

    basicModel.setTitle(originalTitle);
    basicModel.setDescription(originalDescription);
    basicModel.appendKeywords(originalKeywords);

    auto flags = Common::SearchFlags::Metadata | Common::SearchFlags::WholeWords;

    bool replaceSucceeded = basicModel.replace("test", "Replaced", flags);
    QVERIFY(replaceSucceeded);
    QCOMPARE(basicModel.getTitle(), QLatin1String("Replaced title Replaced here"));
    QCOMPARE(basicModel.getDescription(), QLatin1String("Testdescription Replaced"));
    QCOMPARE(basicModel.getKeywordAt(0), QLatin1String("Replaced keyword1"));
    QCOMPARE(basicModel.getKeywordAt(1), originalKeywords[1]);
    QCOMPARE(basicModel.getKeywordAt(2), QLatin1String("keyTestword3 Replaced"));
}

void BasicKeywordsModelTests::replaceKeywordsWithRemoveTest() {
    Artworks::BasicMetadataModel basicModel(m_SpellCheckInfo);

    QString originalTitle = "Test title here";
    QString originalDescription = "Test description";
    QStringList originalKeywords;
    originalKeywords << "Test keyword1" << "keywTestord2" << "keyword3 Test" << "Replaced keyword1";

    basicModel.setTitle(originalTitle);
    basicModel.setDescription(originalDescription);
    basicModel.appendKeywords(originalKeywords);

    auto flags = Common::SearchFlags::Metadata;

    bool replaceSucceeded = basicModel.replace("Test", "Replaced", flags);
    QVERIFY(replaceSucceeded);
    QCOMPARE(basicModel.getTitle(), QLatin1String("Replaced title here"));
    QCOMPARE(basicModel.getDescription(), QLatin1String("Replaced description"));
    QCOMPARE(basicModel.getKeywordsCount(), originalKeywords.length() - 1);
    QCOMPARE(basicModel.getKeywordAt(0), QLatin1String("keywReplacedord2"));
    QCOMPARE(basicModel.getKeywordAt(1), QLatin1String("keyword3 Replaced"));
    QCOMPARE(basicModel.getKeywordAt(2), QLatin1String("Replaced keyword1"));
}

void BasicKeywordsModelTests::removeKeywordsFromSetTest() {
    Artworks::BasicMetadataModel basicModel(m_SpellCheckInfo);

    QStringList originalKeywords;
    originalKeywords << "keyword1" << "keyword2" << "keyword3 Test";
    basicModel.appendKeywords(originalKeywords);
    const bool caseSensitive = false;

    bool removed = basicModel.removeKeywords(QSet<QString>() << "keyword1" << "keyword2" << "keyword3", caseSensitive);
    QVERIFY(removed);

    QCOMPARE(basicModel.getKeywordsCount(), 1);
    QCOMPARE(basicModel.getKeywordAt(0), originalKeywords.back());
}

void BasicKeywordsModelTests::noneKeywordsRemovedFromSetTest() {
    Artworks::BasicMetadataModel basicModel(m_SpellCheckInfo);

    QStringList originalKeywords;
    originalKeywords << "akeyword1" << "keyword2_" << "keyword3 Test";
    basicModel.appendKeywords(originalKeywords);
    const bool caseSensitive = false;

    bool removed = basicModel.removeKeywords(QSet<QString>() << "keyword1" << "keyword2" << "keyword3", caseSensitive);
    QVERIFY(!removed);

    QCOMPARE(basicModel.getKeywordsCount(), originalKeywords.length());
}

void BasicKeywordsModelTests::removeKeywordsCaseSensitiveTest() {
    Artworks::BasicMetadataModel basicModel(m_SpellCheckInfo);

    QStringList originalKeywords;
    originalKeywords << "Keyword1" << "keyworD2" << "keyWord3";
    basicModel.appendKeywords(originalKeywords);
    const bool caseSensitive = true;

    bool removed = basicModel.removeKeywords(QSet<QString>() << "keyword1" << "keyword2" << "keyWord3", caseSensitive);
    QVERIFY(removed);

    QCOMPARE(basicModel.getKeywordsCount(), originalKeywords.length() - 1);
}


#include "warningscheck_tests.h"

#include <memory>
#include <vector>

#include <QSet>
#include <QStringList>

#include "Artworks/basickeywordsmodel.h"
#include "Common/flags.h"
#include "Services/SpellCheck/spellcheckinfo.h"
#include "Services/Warnings/warningsitem.h"

#include "Mocks/artworkmetadatamock.h"
#include "Mocks/warningssettingsmock.h"
#include "stringhelpersfortests.h"

void WarningsCheckTests::emptyKeywordsTest() {
    auto artwork = std::make_shared<Mocks::ArtworkMetadataMock>();
    artwork->initialize();
    Mocks::WarningsSettingsMock warningsSettings;

    QVERIFY(artwork->getWarningsFlags() == Common::WarningFlags::None);

    Warnings::WarningsItem(artwork).checkWarnings(warningsSettings);

    QVERIFY(Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::NoKeywords));
    QVERIFY(!Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::TooFewKeywords));
}

void WarningsCheckTests::tooFewKeywordsTest() {
    auto artwork = std::make_shared<Mocks::ArtworkMetadataMock>();
    artwork->initialize();
    Mocks::WarningsSettingsMock warningsSettings;

    artwork->appendKeyword("random");
    QVERIFY(artwork->getWarningsFlags() == Common::WarningFlags::None);

    Warnings::WarningsItem(artwork).checkWarnings(warningsSettings);

    QVERIFY(!Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::NoKeywords));
    QVERIFY(Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::TooFewKeywords));
}

void WarningsCheckTests::appendingKeywordChangesWarningsTest() {
    auto artwork = std::make_shared<Mocks::ArtworkMetadataMock>();
    artwork->initialize();
    Mocks::WarningsSettingsMock warningsSettings;

    Warnings::WarningsItem(artwork).checkWarnings(warningsSettings);

    QVERIFY(Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::NoKeywords));
    QVERIFY(!Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::TooFewKeywords));

    artwork->appendKeyword("random");

    Warnings::WarningsItem(artwork).checkWarnings(warningsSettings);

    QVERIFY(!Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::NoKeywords));
    QVERIFY(Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::TooFewKeywords));
}

void WarningsCheckTests::spellingKeywordsTest() {
    auto artwork = std::make_shared<Mocks::ArtworkMetadataMock>();
    artwork->initialize();
    Mocks::WarningsSettingsMock warningsSettings;

    artwork->appendKeyword("qwerty");
    auto &rawKeywords = artwork->getBasicModel().getRawKeywords();
    rawKeywords[0].m_IsCorrect = false;

    QVERIFY(artwork->getWarningsFlags() == Common::WarningFlags::None);

    Warnings::WarningsItem(artwork).checkWarnings(warningsSettings);

    QVERIFY(Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::SpellErrorsInKeywords));
}

void WarningsCheckTests::spellingKeywordsChangesWhenRemovedTest() {
    auto artwork = std::make_shared<Mocks::ArtworkMetadataMock>();
    artwork->initialize();
    Mocks::WarningsSettingsMock warningsSettings;

    artwork->appendKeyword("qwerty");
    auto &rawKeywords = artwork->getBasicModel().getRawKeywords();
    rawKeywords[0].m_IsCorrect = false;

    QVERIFY(artwork->getWarningsFlags() == Common::WarningFlags::None);

    Warnings::WarningsItem(artwork).checkWarnings(warningsSettings);

    QVERIFY(Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::SpellErrorsInKeywords));
    QVERIFY(!Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::NoKeywords));

    QString dummy;
    artwork->removeLastKeyword(dummy);

    Warnings::WarningsItem(artwork).checkWarnings(warningsSettings);

    QVERIFY(!Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::SpellErrorsInKeywords));
    QVERIFY(Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::NoKeywords));
}

void WarningsCheckTests::spellingDescriptionTest() {
    auto artwork = std::make_shared<Mocks::ArtworkMetadataMock>();
    artwork->initialize();
    Mocks::WarningsSettingsMock warningsSettings;

    artwork->setDescription("valid and crp test");
    artwork->getSpellCheckInfo().setDescriptionErrors(QSet<QString>() << "crp");

    QVERIFY(artwork->getWarningsFlags() == Common::WarningFlags::None);

    Warnings::WarningsItem(artwork).checkWarnings(warningsSettings);

    QVERIFY(Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::SpellErrorsInDescription));
    QVERIFY(!Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::DescriptionIsEmpty));
}

void WarningsCheckTests::spellingDescriptionChangesTest() {
    auto artwork = std::make_shared<Mocks::ArtworkMetadataMock>();
    artwork->initialize();
    Mocks::WarningsSettingsMock warningsSettings;

    artwork->setDescription("valid and crp test");
    artwork->getSpellCheckInfo().setDescriptionErrors(QSet<QString>() << "crp");

    QVERIFY(artwork->getWarningsFlags() == Common::WarningFlags::None);

    Warnings::WarningsItem(artwork).checkWarnings(warningsSettings);

    QVERIFY(Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::SpellErrorsInDescription));
    QVERIFY(!Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::DescriptionIsEmpty));

    artwork->getSpellCheckInfo().clear();

    Warnings::WarningsItem(artwork).checkWarnings(warningsSettings);

    QVERIFY(!Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::SpellErrorsInDescription));
    QVERIFY(!Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::DescriptionIsEmpty));
}

void WarningsCheckTests::emptyDescriptionTest() {
    auto artwork = std::make_shared<Mocks::ArtworkMetadataMock>();
    artwork->initialize();
    Mocks::WarningsSettingsMock warningsSettings;

    QVERIFY(artwork->getWarningsFlags() == Common::WarningFlags::None);

    Warnings::WarningsItem(artwork).checkWarnings(warningsSettings);

    QVERIFY(Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::DescriptionIsEmpty));
    QVERIFY(!Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::DescriptionNotEnoughWords));
}

void WarningsCheckTests::descriptionLengthChangesTest() {
    auto artwork = std::make_shared<Mocks::ArtworkMetadataMock>();
    artwork->initialize();
    Mocks::WarningsSettingsMock warningsSettings;

    QVERIFY(artwork->getWarningsFlags() == Common::WarningFlags::None);

    Warnings::WarningsItem(artwork).checkWarnings(warningsSettings);

    QVERIFY(Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::DescriptionIsEmpty));
    QVERIFY(!Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::DescriptionNotEnoughWords));

    artwork->setDescription("two words");
    Warnings::WarningsItem(artwork).checkWarnings(warningsSettings);

    QVERIFY(!Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::DescriptionIsEmpty));
    QVERIFY(Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::DescriptionNotEnoughWords));

    artwork->setDescription("three words now");
    Warnings::WarningsItem(artwork).checkWarnings(warningsSettings);

    QVERIFY(!Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::DescriptionIsEmpty));
    QVERIFY(!Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::DescriptionNotEnoughWords));
}

void WarningsCheckTests::descriptionTooBigTest() {
    auto artwork = std::make_shared<Mocks::ArtworkMetadataMock>();
    artwork->initialize();
    Mocks::WarningsSettingsMock warningsSettings;

    QVERIFY(artwork->getWarningsFlags() == Common::WarningFlags::None);

    artwork->setDescription(getRandomString(warningsSettings.getMaxDescriptionLength() + 1, true));

    Warnings::WarningsItem(artwork).checkWarnings(warningsSettings);

    QVERIFY(Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::DescriptionTooBig));
    QVERIFY(!Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::DescriptionIsEmpty));
    QVERIFY(!Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::DescriptionNotEnoughWords));
}

void WarningsCheckTests::emptyTitleTest() {
    auto artwork = std::make_shared<Mocks::ArtworkMetadataMock>();
    artwork->initialize();
    Mocks::WarningsSettingsMock warningsSettings;

    QVERIFY(artwork->getWarningsFlags() == Common::WarningFlags::None);

    Warnings::WarningsItem(artwork).checkWarnings(warningsSettings);

    QVERIFY(Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::TitleIsEmpty));
    QVERIFY(!Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::TitleNotEnoughWords));
}

void WarningsCheckTests::titleLengthChangesTest() {
    auto artwork = std::make_shared<Mocks::ArtworkMetadataMock>();
    artwork->initialize();
    Mocks::WarningsSettingsMock warningsSettings;

    QVERIFY(artwork->getWarningsFlags() == Common::WarningFlags::None);

    Warnings::WarningsItem(artwork).checkWarnings(warningsSettings);

    QVERIFY(Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::TitleIsEmpty));
    QVERIFY(!Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::TitleNotEnoughWords));

    artwork->setTitle("two words");
    Warnings::WarningsItem(artwork).checkWarnings(warningsSettings);

    QVERIFY(!Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::TitleIsEmpty));
    QVERIFY(Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::TitleNotEnoughWords));

    artwork->setTitle("three words now");
    Warnings::WarningsItem(artwork).checkWarnings(warningsSettings);

    QVERIFY(!Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::TitleIsEmpty));
    QVERIFY(!Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::TitleNotEnoughWords));
}

void WarningsCheckTests::spellingTitleTest() {
    auto artwork = std::make_shared<Mocks::ArtworkMetadataMock>();
    artwork->initialize();
    Mocks::WarningsSettingsMock warningsSettings;

    artwork->setTitle("one two three");
    artwork->getSpellCheckInfo().setTitleErrors(QSet<QString>() << "two");

    QVERIFY(artwork->getWarningsFlags() == Common::WarningFlags::None);

    Warnings::WarningsItem(artwork).checkWarnings(warningsSettings);

    QVERIFY(Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::SpellErrorsInTitle));
    QVERIFY(!Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::TitleIsEmpty));
}

void WarningsCheckTests::spellingTitleChangesWhenRemovedTest() {
    auto artwork = std::make_shared<Mocks::ArtworkMetadataMock>();
    artwork->initialize();
    Mocks::WarningsSettingsMock warningsSettings;

    artwork->setTitle("one two three");
    artwork->getSpellCheckInfo().setTitleErrors(QSet<QString>() << "two");

    QVERIFY(artwork->getWarningsFlags() == Common::WarningFlags::None);

    Warnings::WarningsItem(artwork).checkWarnings(warningsSettings);

    QVERIFY(Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::SpellErrorsInTitle));
    QVERIFY(!Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::TitleIsEmpty));

    artwork->getSpellCheckInfo().clear();

    Warnings::WarningsItem(artwork).checkWarnings(warningsSettings);

    QVERIFY(!Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::SpellErrorsInTitle));
    QVERIFY(!Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::TitleIsEmpty));
}

void WarningsCheckTests::keywordsInDescriptionTest() {
    auto artwork = std::make_shared<Mocks::ArtworkMetadataMock>();
    artwork->initialize();
    Mocks::WarningsSettingsMock warningsSettings;

    artwork->setDescription("one two three");
    artwork->appendKeyword("two");

    Warnings::WarningsItem(artwork).checkWarnings(warningsSettings);

    QVERIFY(Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::KeywordsInDescription));

    QVERIFY(!Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::KeywordsInTitle));
    QVERIFY(!Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::DescriptionIsEmpty));
    QVERIFY(!Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::NoKeywords));
}

void WarningsCheckTests::keywordsInTitleTest() {
    auto artwork = std::make_shared<Mocks::ArtworkMetadataMock>();
    artwork->initialize();
    Mocks::WarningsSettingsMock warningsSettings;

    artwork->setTitle("one two three");
    artwork->appendKeyword("three");

    Warnings::WarningsItem(artwork).checkWarnings(warningsSettings);

    QVERIFY(Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::KeywordsInTitle));

    QVERIFY(!Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::KeywordsInDescription));
    QVERIFY(!Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::TitleIsEmpty));
    QVERIFY(!Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::NoKeywords));
}

void WarningsCheckTests::titleTooBigTest() {
    auto artwork = std::make_shared<Mocks::ArtworkMetadataMock>();
    artwork->initialize();
    Mocks::WarningsSettingsMock warningsSettings;

    QStringList titleWords;
    int words = warningsSettings.getMaxTitleWords() + 1;
    while (words--) { titleWords.append(getRandomString(10)); }
    artwork->setTitle(titleWords.join(' '));

    Warnings::WarningsItem(artwork).checkWarnings(warningsSettings);

    QVERIFY(Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::TitleTooManyWords));
    QVERIFY(!Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::TitleIsEmpty));
    QVERIFY(!Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::TitleNotEnoughWords));
}

void WarningsCheckTests::descriptionTooBigChangesTest() {
    auto artwork = std::make_shared<Mocks::ArtworkMetadataMock>();
    artwork->initialize();
    Mocks::WarningsSettingsMock warningsSettings;

    QVERIFY(artwork->getWarningsFlags() == Common::WarningFlags::None);

    artwork->setDescription(getRandomString(warningsSettings.getMaxDescriptionLength() + 1, true));

    Warnings::WarningsItem(artwork).checkWarnings(warningsSettings);

    QVERIFY(Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::DescriptionTooBig));

    artwork->setDescription(getRandomString(warningsSettings.getMaxDescriptionLength() - 1, true));

    Warnings::WarningsItem(artwork).checkWarnings(warningsSettings);

    QVERIFY(!Common::HasFlag(artwork->getWarningsFlags(), Common::WarningFlags::DescriptionTooBig));
}

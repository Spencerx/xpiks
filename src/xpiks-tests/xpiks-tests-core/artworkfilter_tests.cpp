#include "artworkfilter_tests.h"
#include "Mocks/artworkmetadatamock.h"
#include "../../xpiks-qt/Helpers/filterhelpers.h"

void ArtworkFilterTests::searchImageVectorTest() {
    auto artwork = std::make_shared<Mocks::ArtworkMetadataMock>("/path/to/file.jpg");
    artwork->initAsEmpty();
    QVERIFY(Helpers::hasSearchMatch("x:image", artwork, flagsAnyTermWithFilepath()));
    QVERIFY(!Helpers::hasSearchMatch("x:vector", artwork, flagsAnyTermWithFilepath()));

    artwork->attachVector("/path/to/file.eps");
    QVERIFY(!Helpers::hasSearchMatch("x:image", artwork, flagsAnyTermWithFilepath()));
    QVERIFY(Helpers::hasSearchMatch("x:vector", artwork, flagsAnyTermWithFilepath()));
}

void ArtworkFilterTests::searchByKeywordsTest() {
    auto artwork = std::make_shared<Mocks::ArtworkMetadataMock>("/path/to/file.jpg");
    artwork->initAsEmpty();
    artwork->setKeywords(QStringList() << "keyword" << "another" << "test");
    QVERIFY(Helpers::hasSearchMatch("keyw TSTS", artwork, flagsAnyTermWithFilepath()));
    QVERIFY(!Helpers::hasSearchMatch("!keyw", artwork, flagsAnyTermWithFilepath()));
    QVERIFY(Helpers::hasSearchMatch("!keyword", artwork, flagsAnyTermWithFilepath()));

    QVERIFY(Helpers::hasSearchMatch("keyword super", artwork, flagsAnyTermWithFilepath()));
    QVERIFY(!Helpers::hasSearchMatch("keyword super", artwork, flagsAllTermsWithoutFilepath()));
}

void ArtworkFilterTests::searchByTitleTest() {
    auto artwork = std::make_shared<Mocks::ArtworkMetadataMock>("/path/to/file.jpg");
    artwork->initAsEmpty();
    artwork->setTitle("my long title here");
    QVERIFY(Helpers::hasSearchMatch("tit", artwork, flagsAnyTermWithFilepath()));
    QVERIFY(!Helpers::hasSearchMatch("!tit", artwork, flagsAnyTermWithFilepath()));
    // strict search works only for keywords
    QVERIFY(!Helpers::hasSearchMatch("!title", artwork, flagsAnyTermWithFilepath()));

    QVERIFY(!Helpers::hasSearchMatch("keyword super", artwork, flagsAnyTermWithFilepath()));
    QVERIFY(Helpers::hasSearchMatch("here my", artwork, flagsAllTermsWithoutFilepath()));
}

void ArtworkFilterTests::searchByDescriptionTest() {
    auto artwork = std::make_shared<Mocks::ArtworkMetadataMock>("/path/to/file.jpg");
    artwork->initAsEmpty();
    artwork->setDescription("my long desciption john");
    QVERIFY(Helpers::hasSearchMatch("o", artwork, flagsAnyTermWithFilepath()));
    QVERIFY(!Helpers::hasSearchMatch("!o", artwork, flagsAnyTermWithFilepath()));
    // strict search works only for keywords
    QVERIFY(!Helpers::hasSearchMatch("!description", artwork, flagsAnyTermWithFilepath()));

    QVERIFY(!Helpers::hasSearchMatch("myjohn", artwork, flagsAnyTermWithFilepath()));
    QVERIFY(!Helpers::hasSearchMatch("descriptionmy", artwork, flagsAllTermsWithoutFilepath()));
}

void ArtworkFilterTests::searchByFilepathTest() {
    auto artwork = std::make_shared<Mocks::ArtworkMetadataMock>("/path/to/file.jpg");
    artwork->initAsEmpty();
    QVERIFY(Helpers::hasSearchMatch("file.jpg", artwork, flagsAnyTermWithFilepath()));
    QVERIFY(!Helpers::hasSearchMatch("file.jpg", artwork, flagsAllTermsWithoutFilepath()));
}

void ArtworkFilterTests::strictSearchTest() {
    auto artwork = std::make_shared<Mocks::ArtworkMetadataMock>("/path/to/file.jpg");
    artwork->initAsEmpty();
    artwork->setKeywords(QStringList() << "keyword" << "ano!ther" << "test" << "k");
    QVERIFY(Helpers::hasSearchMatch("keyw", artwork, flagsAnyTermWithFilepath()));
    QVERIFY(!Helpers::hasSearchMatch("!keyw", artwork, flagsAnyTermWithFilepath()));
    QVERIFY(Helpers::hasSearchMatch("!keyword", artwork, flagsAnyTermWithFilepath()));

    QVERIFY(!Helpers::hasSearchMatch("!!", artwork, flagsAnyTermWithFilepath()));
    QVERIFY(Helpers::hasSearchMatch("!k", artwork, flagsAllTermsWithoutFilepath()));

    artwork->appendKeyword("!");
    QVERIFY(Helpers::hasSearchMatch("!", artwork, flagsAnyTermWithFilepath()));
}

void ArtworkFilterTests::searchWithSpaceTest() {
    auto artwork = std::make_shared<Mocks::ArtworkMetadataMock>("/path/to/file.jpg");
    artwork->initAsEmpty();

    artwork->setTitle("A vector here");
    artwork->setDescription("Vector there");
    auto flags = Common::SearchFlags::IncludeSpaces | Common::SearchFlags::Metadata;

    QVERIFY(Helpers::hasSearchMatch(" vector ", artwork, flags));

    artwork->setTitle("");
    QVERIFY(!Helpers::hasSearchMatch(" vector ", artwork, flags));
}

void ArtworkFilterTests::caseSensitiveKeywordSearchTest() {
    auto artwork = std::make_shared<Mocks::ArtworkMetadataMock>("/path/to/file.jpg");
    artwork->initAsEmpty();
    artwork->setKeywords(QStringList() << "keYwOrd" << "keYword");

    auto flags = Common::SearchFlags::Keywords | Common::SearchFlags::CaseSensitive;

    QVERIFY(Helpers::hasSearchMatch("YwO", artwork, flags));
    QVERIFY(!Helpers::hasSearchMatch("ywO", artwork, flags));
}

void ArtworkFilterTests::cantFindWithFilterDescriptionTest() {
    auto artwork = std::make_shared<Mocks::ArtworkMetadataMock>("/path/to/file.jpg");
    artwork->initAsEmpty();
    artwork->setDescription("token between here");
    artwork->setKeywords(QStringList() << "some keyword" << "another stuff");

    auto flags = Common::SearchFlags::Keywords | Common::SearchFlags::Title;

    QVERIFY(!Helpers::hasSearchMatch("between", artwork, flags));
    QVERIFY(Helpers::hasSearchMatch("between", artwork, flags | Common::SearchFlags::Description));
}

void ArtworkFilterTests::cantFindWithFilterTitleTest() {
    auto artwork = std::make_shared<Mocks::ArtworkMetadataMock>("/path/to/file.jpg");
    artwork->initAsEmpty();
    artwork->setTitle("token between here");
    artwork->setKeywords(QStringList() << "some keyword" << "another stuff");

    auto flags = Common::SearchFlags::Description | Common::SearchFlags::Keywords;

    QVERIFY(!Helpers::hasSearchMatch("between", artwork, flags));
    QVERIFY(Helpers::hasSearchMatch("between", artwork, flags | Common::SearchFlags::Title));
}

void ArtworkFilterTests::cantFindWithFilterKeywordsTest() {
    auto artwork = std::make_shared<Mocks::ArtworkMetadataMock>("/path/to/file.jpg");
    artwork->initAsEmpty();
    artwork->setDescription("another keyword in description");
    artwork->setTitle("token between here");
    artwork->setKeywords(QStringList() << "some keyword" << "another stuff");

    auto flags = Common::SearchFlags::Description | Common::SearchFlags::Title;

    QVERIFY(!Helpers::hasSearchMatch("stuff", artwork, flags));
    QVERIFY(Helpers::hasSearchMatch("stuff", artwork, flags | Common::SearchFlags::Keywords));
}

void ArtworkFilterTests::cantFindWithFilterSpecialTest() {
    auto artwork = std::make_shared<Mocks::ArtworkMetadataMock>("/path/to/file.jpg");
    artwork->initAsEmpty();
    artwork->setDescription("another keyword in description");
    artwork->setTitle("token between here");
    artwork->setKeywords(QStringList() << "some keyword" << "another stuff");

    auto flags = Common::SearchFlags::Description | Common::SearchFlags::Title | Common::SearchFlags::Keywords;

    QVERIFY(!Helpers::hasSearchMatch("x:modified", artwork, flags));
    QVERIFY(Helpers::hasSearchMatch("x:modified", artwork, flags | Common::SearchFlags::ReservedTerms));
}

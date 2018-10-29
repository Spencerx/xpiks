#ifndef ARTWORKFILTERTESTS_H
#define ARTWORKFILTERTESTS_H

#include <QObject>
#include <QtTest>  // IWYU pragma: keep
// IWYU pragma: no_include <QString>

#include "Common/flags.h"

class ArtworkFilterTests : public QObject
{
    Q_OBJECT
private:
    Common::SearchFlags flagsAnyTermWithFilepath() { return Common::SearchFlags::AnyTermsEverything; }
    Common::SearchFlags flagsAllTermsWithoutFilepath() {
        Common::SearchFlags flags = Common::SearchFlags::AllTermsEverything;
        Common::UnsetFlag(flags, Common::SearchFlags::Filepath);
        return flags;
    }

private slots:
    void searchImageVectorTest();
    void searchByKeywordsTest();
    void searchByTitleTest();
    void searchByDescriptionTest();
    void searchByFilepathTest();
    void strictSearchTest();
    void searchWithSpaceTest();
    void caseSensitiveKeywordSearchTest();
    void cantFindWithFilterDescriptionTest();
    void cantFindWithFilterTitleTest();
    void cantFindWithFilterKeywordsTest();
    void cantFindWithFilterSpecialTest();
};

#endif // ARTWORKFILTERTESTS_H

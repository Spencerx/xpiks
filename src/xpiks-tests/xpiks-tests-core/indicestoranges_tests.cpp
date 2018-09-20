#include "indicestoranges_tests.h"
#include <Helpers/indicesranges.h>
#include <Helpers/indiceshelper.h>

#include <QDebug>
#include <QVector>
#include <cstdarg>
#include <string>
#include <utility>
#include <vector>

using Pair = std::pair<int, int>;
using Pairs = std::vector<Pair>;
using Indices = std::vector<int>;

using IntArray = QVector<int>;
using ChunksArray = QVector<IntArray>;

Pairs MAKE_PAIRS(int length, ...) {
    Pairs pairs;

    va_list args;
    va_start(args, length);

    for (int i = 0; i < length; ++i) {
        int first = va_arg(args, int);
        int second = va_arg(args, int);

        pairs.emplace_back(Pair(first, second));
    }

    va_end(args);

    return pairs;
}

void COMPARE_PAIRS(const Pairs &actual, const Pairs &expected) {
    if (actual.size() == expected.size()) {
        const size_t actualLength = actual.size();

        for (int i = 0; i < actualLength; ++i) {
            const Pair &actualPair = actual.at(i);
            const Pair &expectedPair = expected.at(i);

            if ((actualPair.first != expectedPair.first) ||
                    (actualPair.second != expectedPair.second)) {
                QFAIL(QString("(%1, %2) was expected, but (%3, %4) found")
                      .arg(expectedPair.first)
                      .arg(expectedPair.second)
                      .arg(actualPair.first)
                      .arg(actualPair.second)
                      .toStdString().c_str());
                break;
            }
        }
    } else {
        QFAIL(QString("Length was different. %1 received but %2 expected")
              .arg(actual.size())
              .arg(expected.size())
              .toStdString().c_str());
    }
}

void IndicesToRangesTests::noItemsToEmptyIndicesTest() {
    Indices indices;
    Helpers::IndicesRanges ranges(indices);

    Pairs expectedPairs;

    COMPARE_PAIRS(ranges.getRanges(), expectedPairs);
}

void IndicesToRangesTests::allRangeIndicesTest() {
    Indices indices = { 0, 1, 2, 3, 4, 5 };
    auto actualPairs = Helpers::IndicesRanges(indices).getRanges();

    Pairs expectedPairs = MAKE_PAIRS(1, 0, 5);
    COMPARE_PAIRS(actualPairs, expectedPairs);
}

void IndicesToRangesTests::allRangeNotFromZeroTest() {
    Indices indices = { 1, 2, 3, 4, 5 };
    auto actualPairs = Helpers::IndicesRanges(indices).getRanges();

    Pairs expectedPairs = MAKE_PAIRS(1, 1, 5);
    COMPARE_PAIRS(actualPairs, expectedPairs);
}

void IndicesToRangesTests::allRangeOnlyTwoItemsTest() {
    Indices indices = { 4, 5 };
    auto actualPairs = Helpers::IndicesRanges(indices).getRanges();

    Pairs expectedPairs = MAKE_PAIRS(1, 4, 5);
    COMPARE_PAIRS(actualPairs, expectedPairs);
}

void IndicesToRangesTests::allRangeOnlyOneItemNotZeroTest() {
    Indices indices = { 4 };
    auto actualPairs = Helpers::IndicesRanges(indices).getRanges();

    Pairs expectedPairs = MAKE_PAIRS(1, 4, 4);
    COMPARE_PAIRS(actualPairs, expectedPairs);
}

void IndicesToRangesTests::allRangeOnlyOneItemZeroTest() {
    Indices indices = { 0 };
    auto actualPairs = Helpers::IndicesRanges(indices).getRanges();

    Pairs expectedPairs = MAKE_PAIRS(1, 0, 0);
    COMPARE_PAIRS(actualPairs, expectedPairs);
}

void IndicesToRangesTests::separateRangesSimpleTest() {
    Indices indices = { 1, 2, 4, 5 };
    auto actualPairs = Helpers::IndicesRanges(indices).getRanges();

    Pairs expectedPairs = MAKE_PAIRS(2, 1, 2, 4, 5);
    COMPARE_PAIRS(actualPairs, expectedPairs);
}

void IndicesToRangesTests::separateRangesWithOneItemInTheBeginningTest() {
    Indices indices = { 1, 4, 5 };
    auto actualPairs = Helpers::IndicesRanges(indices).getRanges();

    Pairs expectedPairs = MAKE_PAIRS(2, 1, 1, 4, 5);
    COMPARE_PAIRS(actualPairs, expectedPairs);
}

void IndicesToRangesTests::separateRangesWithOneItemInTheMiddleTest() {
    Indices indices = { 0, 1, 3, 5, 6 };
    auto actualPairs = Helpers::IndicesRanges(indices).getRanges();

    Pairs expectedPairs = MAKE_PAIRS(3, 0, 1, 3, 3, 5, 6);
    COMPARE_PAIRS(actualPairs, expectedPairs);
}

void IndicesToRangesTests::separateRangesWithOneItemInTheEndTest() {
    Indices indices = { 0, 1, 2, 3, 6 };
    auto actualPairs = Helpers::IndicesRanges(indices).getRanges();

    Pairs expectedPairs = MAKE_PAIRS(2, 0, 3, 6, 6);
    COMPARE_PAIRS(actualPairs, expectedPairs);
}

void IndicesToRangesTests::combinedTest() {
    Indices indices = { 0, 1, 2, 4, 6, 7, 10, 13 };
    auto actualPairs = Helpers::IndicesRanges(indices).getRanges();

    Pairs expectedPairs = MAKE_PAIRS(5, 0, 2, 4, 4, 6, 7, 10, 10, 13, 13);
    COMPARE_PAIRS(actualPairs, expectedPairs);
}

void IndicesToRangesTests::emptyArrayToChunksTest() {
    IntArray array;
    ChunksArray chunks;
    int result = Helpers::splitIntoChunks<int>(array, 3, chunks);
    QCOMPARE(result, chunks.size());

    QVERIFY(chunks.isEmpty());
}

void IndicesToRangesTests::trivialArrayToChunksTest() {
    IntArray array = IntArray() << 0;
    ChunksArray chunks;
    int result = Helpers::splitIntoChunks<int>(array, 1, chunks);
    QCOMPARE(result, chunks.size());

    QVERIFY(chunks.length() == 1);
    QVERIFY(chunks[0] == array);
}

void IndicesToRangesTests::splitWithOneItemInChunkTest() {
    IntArray array = IntArray() << 0 << 1 << 2 << 4 << 6 << 7 << 10 << 13;
    ChunksArray chunks;
    int result = Helpers::splitIntoChunks<int>(array, array.size(), chunks);
    QCOMPARE(result, chunks.size());

    QCOMPARE(chunks.size(), array.size());
    for (int i = 0; i < array.size(); ++i) {
        QVERIFY(chunks[i].size() == 1);
        QCOMPARE(chunks[i][0], array[i]);
    }
}

void IndicesToRangesTests::splitEvenEasyCaseTest() {
    IntArray array = IntArray() << 0 << 1 << 2 << 3 << 4 << 5 << 6 << 7;
    ChunksArray chunks;
    int result = Helpers::splitIntoChunks<int>(array, 4, chunks);
    QCOMPARE(result, chunks.size());

    QCOMPARE(chunks.size(), 4);
    for (int i = 0; i < 4; ++i) {
        QVERIFY(chunks[i].size() == 2);

        QCOMPARE(chunks[i][0], i*2);
        QCOMPARE(chunks[i][1], i*2 + 1);
    }
}

void IndicesToRangesTests::splitOddEasyCaseTest() {
    IntArray array = IntArray() << 0 << 1 << 2 << 3 << 4 << 5 << 6;
    ChunksArray chunks;
    int result = Helpers::splitIntoChunks<int>(array, 4, chunks);
    QCOMPARE(result, chunks.size());

    QCOMPARE(chunks.size(), 4);
    for (int i = 0; i < 3; ++i) {
        QVERIFY(chunks[i].size() == 2);

        QCOMPARE(chunks[i][0], i*2);
        QCOMPARE(chunks[i][1], i*2 + 1);
    }

    QVERIFY(chunks[3].size() == 1);
    QCOMPARE(chunks[3][0], 6);
}

void IndicesToRangesTests::splitIntoOnePieceTest() {
    IntArray array = IntArray() << 0 << 1 << 2 << 4 << 6 << 7 << 10 << 13;
    ChunksArray chunks;
    int result = Helpers::splitIntoChunks<int>(array, 1, chunks);
    QCOMPARE(result, chunks.size());

    QCOMPARE(chunks.size(), 1);
    for (int i = 0; i < array.size(); ++i) {
        QCOMPARE(chunks[0][i], array[i]);
    }
}

void IndicesToRangesTests::splitIntoZeroPiecesTest() {
    IntArray array = IntArray() << 0 << 1 << 2 << 4 << 6 << 7 << 10 << 13;
    ChunksArray chunks;
    int result = Helpers::splitIntoChunks<int>(array, 0, chunks);
    QCOMPARE(result, chunks.size());

    QCOMPARE(chunks.size(), 0);
}

void IndicesToRangesTests::splitIntoMoreThanASizeTest() {
    IntArray array = IntArray() << 0 << 1 << 2 << 4 << 6 << 7 << 10 << 13;
    ChunksArray chunks;
    int result = Helpers::splitIntoChunks<int>(array, array.size() + 1, chunks);
    QCOMPARE(result, chunks.size());

    QCOMPARE(chunks.size(), 0);
}

void IndicesToRangesTests::splitIntoTwoUnevenChunksTest() {
    IntArray array = IntArray() << 0 << 1 << 2 << 3 << 4 << 5 << 6;
    ChunksArray chunks;
    int result = Helpers::splitIntoChunks<int>(array, 2, chunks);
    QCOMPARE(result, chunks.size());

    QCOMPARE(chunks.size(), 2);

    QCOMPARE(chunks[0].size(), 4);
    for (int i = 0; i < 4; ++i) {
        QCOMPARE(chunks[0][i], array[i]);
    }

    QCOMPARE(chunks[1].size(), 3);
    for (int i = 4; i < array.size(); ++i) {
        QCOMPARE(chunks[1][i - 4], array[i]);
    }
}

void IndicesToRangesTests::splitIntoMoreThanAHalfTest() {
    IntArray array = IntArray() << 0 << 1 << 2 << 3 << 4 << 5;
    ChunksArray chunks;
    int result = Helpers::splitIntoChunks<int>(array, 4, chunks);
    QCOMPARE(result, chunks.size());

    QCOMPARE(chunks.size(), 3);

    for (int i = 0; i < 3; ++i) {
        QVERIFY(chunks[i].size() == 2);

        QCOMPARE(chunks[i][0], i*2);
        QCOMPARE(chunks[i][1], i*2 + 1);
    }
}

void IndicesToRangesTests::sameNumbersTest() {
    Indices indices = { 0, 1, 1, 1, 6 };
    auto actualPairs = Helpers::IndicesRanges(indices).getRanges();

    Pairs expectedPairs = MAKE_PAIRS(2, 0, 1, 6, 6);
    COMPARE_PAIRS(actualPairs, expectedPairs);
}

void IndicesToRangesTests::allSameNumbersTest() {
    Indices indices = { 0, 0, 0, 0, 0 };
    auto actualPairs = Helpers::IndicesRanges(indices).getRanges();

    Pairs expectedPairs = MAKE_PAIRS(1, 0, 0);
    COMPARE_PAIRS(actualPairs, expectedPairs);
}

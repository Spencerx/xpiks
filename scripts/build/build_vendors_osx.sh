#!/bin/bash
BUILD_MODE=${1}

# to clean run build_vendors_osx.sh debug clean

TRAVIS_CI=

case ${BUILD_MODE} in
    release)
        TARGET="release"
        ;;
    debug|travis-ci)
        TARGET="debug"
        TRAVIS_CI=travis-ci
        ;;
    fullrelease)
        TARGET="release"
        ;;
    fulldebug)
        TARGET="debug"
        ;;
    *)
        echo "$(basename ${0}) error: Invalid command arguments"
        echo "    usage: $(basename ${0}) <debug|release|travis-ci> [optional make flags]"
        exit 1
        ;;
esac

echo "Build target is: $TARGET"

if [ -n "$TRAVIS_CI" ]; then
    QT_BIN_DIR=/usr/local/opt/qt/bin
else
    QT_BIN_DIR=~/Qt5.9.7/5.9.7/clang_64/bin/
fi

#export LDFLAGS="-L/usr/local/opt/qt/lib"
#export CPPFLAGS="-I/usr/local/opt/qt/include"
echo -e "${PRINT_PREFIX} Using ${QT_BIN_DIR} as Qt bin directory path"
export PATH="${PATH}:${QT_BIN_DIR}"

shift
MAKE_FLAGS="-j$(sysctl -n hw.ncpu) ${*}" # do we need argument validation?

### output prefix
GREEN_COLOR='\033[0;32m'
RESET_COLOR='\033[0m'
PRINT_PREFIX="${GREEN_COLOR}[VENDORS]${RESET_COLOR}"

echo -e "${PRINT_PREFIX} Start vendors preparation for ${BUILD_MODE} with make flags: ${MAKE_FLAGS}"

### find root dir
ROOT_DIR=$(git rev-parse --show-toplevel)
cd ${ROOT_DIR}

### translations
if [ ${BUILD_MODE} == "release" ] || [ ${BUILD_MODE} == "fullrelease" ]; then
    echo -e "${PRINT_PREFIX} Generating translations..."
    cd ${ROOT_DIR}/src/xpiks-qt/deps/translations/
    make ${MAKE_FLAGS}
    echo -e "${PRINT_PREFIX} Generating translations... - done."
fi

if [ ${BUILD_MODE} == "fulldebug" ] || [ ${BUILD_MODE} == "fullrelease" ]
then
    echo "Full mode"
    ### libthmbnlr
    echo -e "${PRINT_PREFIX} Building libthmbnlr..."
    cd ${ROOT_DIR}/../libthmbnlr/src/libthmbnlr
    qmake "CONFIG+=${TARGET}" libthmbnlr.pro
    make ${MAKE_FLAGS}	
    cp libthmbnlr.a ${ROOT_DIR}/libs/${TARGET}
    echo -e "${PRINT_PREFIX} Building libthmbnlr... - done."
		
    ### libxpks
    echo -e "${PRINT_PREFIX} Building libxpks..."
    cd ${ROOT_DIR}/../libxpks/src/xpks
    qmake "CONFIG+=${TARGET}" xpks.pro
    make ${MAKE_FLAGS}
    cp libxpks.a ${ROOT_DIR}/libs/${TARGET}
    echo -e "${PRINT_PREFIX} Building libxpks... - done."
else
    echo "Normal mode"
    ### libthmbnlr
    echo -e "${PRINT_PREFIX} Building libthmbnlr..."
    cd ${ROOT_DIR}/vendors/libthmbnlr
    qmake "CONFIG+=${TARGET}" thmbnlr.pro
    make ${MAKE_FLAGS}
    echo -e "${PRINT_PREFIX} Building libthmbnlr... - done."
		
    ### libxpks
    echo -e "${PRINT_PREFIX} Building libxpks..."
    cd ${ROOT_DIR}/src/libxpks_stub
    qmake "CONFIG+=${TARGET}" libxpks_stub.pro
    make ${MAKE_FLAGS}
    cp libxpks.a ${ROOT_DIR}/libs/${TARGET}
    echo -e "${PRINT_PREFIX} Building libxpks... - done."
fi

### tiny-aes
echo -e "${PRINT_PREFIX} Renaming tiny-aes..."
cd ${ROOT_DIR}/vendors/tiny-aes
mv aes.c aes.cpp
echo -e "${PRINT_PREFIX} Renaming tiny-aes... - done."

### cpp-libface
echo -e "${PRINT_PREFIX} Building cpp-libface..."
cd ${ROOT_DIR}/vendors/cpp-libface/libface-project
qmake "CONFIG+=${TARGET}" libface.pro
make ${MAKE_FLAGS}
cp -v -a libface.*dylib ${ROOT_DIR}/libs/${TARGET}
echo -e "${PRINT_PREFIX} Building cpp-libface... - done."

### chillout
echo -e "${PRINT_PREFIX} Building chillout..."
cd ${ROOT_DIR}/vendors/chillout/src/chillout
qmake "CONFIG+=${TARGET}" chillout.pro
make ${MAKE_FLAGS}
cp -v libchillout.a ${ROOT_DIR}/libs/${TARGET}
echo -e "${PRINT_PREFIX} Building chillout... - done."

### ssdll
echo -e "${PRINT_PREFIX} Building ssdll..."
cd ${ROOT_DIR}/vendors/ssdll/src/ssdll/
qmake "CONFIG+=${TARGET}" ssdll.pro
make ${MAKE_FLAGS}
cp -v -a libssdll.*dylib ${ROOT_DIR}/libs/${TARGET}
echo -e "${PRINT_PREFIX} Building ssdll... - done."

### quazip
echo -e "${PRINT_PREFIX} Building quazip..."
cd ${ROOT_DIR}/vendors/quazip/quazip/
qmake "CONFIG+=${TARGET}" quazip.pro
make ${MAKE_FLAGS}
cp -v -a libquazip.*dylib ${ROOT_DIR}/libs/${TARGET}
echo -e "${PRINT_PREFIX} Building quazip... - done."

### hunspell
echo -e "${PRINT_PREFIX} Building hunspell..."
cd ${ROOT_DIR}/vendors/hunspell/
qmake "CONFIG+=${TARGET}" hunspell.pro
make ${MAKE_FLAGS}
echo -e "${PRINT_PREFIX} Building hunspell... - done."

### recoverty
echo -e "${PRINT_PREFIX} Building recoverty..."
cd ${ROOT_DIR}/src/recoverty/
qmake "CONFIG+=${TARGET} ${TRAVIS_CI}" recoverty.pro
make ${MAKE_FLAGS}
echo -e "${PRINT_PREFIX} Building recoverty... - done."

echo -e "${PRINT_PREFIX} Vendors preparation for ${BUILD_MODE}: done."

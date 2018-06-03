#!/bin/bash

APP_NAME=Xpiks
VERSION="1.5.2"
APPDIR_NAME="Xpiks"

STAGING_DIR="./${APP_NAME}.AppDir"

DEPLOY_TOOL=linuxdeploy

BUILD_DIR="../../build-xpiks-qt-Desktop_Qt_5_6_2_GCC_64bit-Release"
LIBS_PROFILE=release

XPIKS_QT_DIR=".."
REPO_ROOT="../../.."

QT_BIN_PATH=/home/ktar/Qt5.6.2/5.6.2/gcc_64/bin

echo "------------------------------"
echo "Working in $(pwd)"
echo "------------------------------"

# clear out any old data
echo "Wiping old data..."
rm -v -rf "${STAGING_DIR}"
rm -v *.AppImage

QML_IMPORTS="-qmldir $XPIKS_QT_DIR/ -qmldir $XPIKS_QT_DIR/Components/ -qmldir $XPIKS_QT_DIR/Constants/ -qmldir $XPIKS_QT_DIR/Dialogs/ -qmldir $XPIKS_QT_DIR/StyledControls/ -qmldir $XPIKS_QT_DIR/StackViews/ -qmldir $XPIKS_QT_DIR/CollapserTabs/"

$DEPLOY_TOOL -appdir "$STAGING_DIR" -blacklist linuxdeploy.blacklist -exe "${BUILD_DIR}/$APP_NAME" -icon "$XPIKS_QT_DIR/debian/xpiks.png" -libs "$REPO_ROOT/libs/$LIBS_PROFILE" -out appimage -overwrite -qmake "$QT_BIN_PATH/qmake" $QML_IMPORTS

# -------------------------------------------------
# ---------- Manual dependencies section-----------
# -------------------------------------------------

cp -v -r "${XPIKS_QT_DIR}"/deps/* "${STAGING_DIR}/"
rm -v $STAGING_DIR/translations/*.ts
rm -v $STAGING_DIR/dmg-background.jpg

cat << EOF > "$STAGING_DIR/$APP_NAME.desktop"
[Desktop Entry]
Type=Application
Name=$APP_NAME
GenericName=Xpiks
Exec=./AppRun %F
Icon=xpiks
Comment=Cross-platform (X) Photo Keywording Software
Terminal=true
StartupNotify=true
Encoding=UTF-8
Categories=Graphics;Photography;
EOF

chmod +x "$STAGING_DIR/$APP_NAME.desktop"

mkdir $STAGING_DIR/ac_sources
mv $STAGING_DIR/en_wordlist.tsv $STAGING_DIR/ac_sources/

appimagetool-x86_64.AppImage --verbose -n "$STAGING_DIR" "$APP_NAME-v$VERSION-x86_64.AppImage"

echo "Done"

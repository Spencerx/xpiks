#!/bin/bash

# Can set variables 
# $BUILD_MODE (debug/release), 
# $BUILD_DIR - path to build directory with executable
# $DEPLOY_TOOL - path to macdeployqt
# $VERSION - default (1.5.2)
# $APP_NAME - default (Xpiks)

dir_resolve()
{
    cd "$1" 2>/dev/null || return $?  # cd to desired directory; if fail, quell any error messages but return exit status
    echo "`pwd -P`" # output full, link-resolved path
}

BUILD_MODE="${BUILD_MODE:-release}"

XPIKS_ROOT=$(git rev-parse --show-toplevel)

XPIKS_DEPS_DIR="${XPIKS_ROOT}/src/xpiks-qt/deps"
XPIKS_QT_DIR="${XPIKS_ROOT}/src/xpiks-qt"

DEFAULT_BUILD_DIR="${XPIKS_ROOT}/src/build-xpiks-qt-Desktop_Qt_5_6_2_clang_64bit-Release"
BUILD_DIR="${BUILD_DIR:-$DEFAULT_BUILD_DIR}"

QT_BIN_DIR=~/Qt5.6.2/5.6/clang_64/bin
DEFAULT_DEPLOY_TOOL="$QT_BIN_DIR/macdeployqt"
DEPLOY_TOOL="${DEPLOY_TOOL:-$DEFAULT_DEPLOY_TOOL}"

LIBS_PATH="${XPIKS_ROOT}/libs/${BUILD_MODE}"

DEFAULT_VERSION="1.5.2"
DEFAULT_APP_NAME="Xpiks"
VOL_NAME="Xpiks"
APP_NAME="${APP_NAME:-$DEFAULT_APP_NAME}"
VERSION="${VERSION:-$DEFAULT_VERSION}"

# ----------------------------------------

if [ ! -f "${XPIKS_DEPS_DIR}/exiftool/exiftool" ]; then
    echo "Exiftool not found! Please put latest production release into the ../deps/ dir"
    exit 1
fi

DMG_BACKGROUND_IMG="dmg-background.jpg"
DMG_BACKGROUND_PATH="${XPIKS_DEPS_DIR}/$DMG_BACKGROUND_IMG"

DMG_TMP="${APP_NAME}-v${VERSION}.tmp.dmg"
DMG_FINAL="${APP_NAME}-v${VERSION}.dmg"

STAGING_DIR="./osx-release-staging"

if [ ! -d "$BUILD_DIR" ]; then
    echo "Build directory not found: $BUILD_DIR"
    exit 1
fi

if [ -d "/Volumes/${VOL_NAME}" ]; then
    echo "Please unmount existing Volume ${VOL_NAME} first!"
    exit 1
fi

rm -v -rf "${STAGING_DIR}" "${DMG_TMP}" "${DMG_FINAL}"
echo "Previous deployment leftovers removed"
mkdir -p "$STAGING_DIR"

sleep 2s

cp -rpfv "$BUILD_DIR/${APP_NAME}.app" "${STAGING_DIR}"

pushd "$STAGING_DIR"

QML_IMPORTS="-qmldir=$XPIKS_QT_DIR/ -qmldir=$XPIKS_QT_DIR/Components/ -qmldir=$XPIKS_QT_DIR/Constants/ -qmldir=$XPIKS_QT_DIR/Dialogs/ -qmldir=$XPIKS_QT_DIR/StyledControls/ -qmldir=$XPIKS_QT_DIR/StackViews/ -qmldir=$XPIKS_QT_DIR/CollapserTabs/"

$DEPLOY_TOOL "${APP_NAME}.app" -no-strip -verbose=2 -executable="${APP_NAME}.app/Contents/MacOS/${APP_NAME}" $QML_IMPORTS

popd

echo "Copying libraries..."

FFMPEG_LIBS=(
    libavcodec.57.dylib
#    libavdevice.57.dylib
    libavfilter.6.dylib
    libavformat.57.dylib
    libavutil.55.dylib
    libswscale.4.dylib
)

LIBS_TO_DEPLOY=(
    libssdll.1.0.0.dylib
    libface.1.0.0.dylib
    libquazip.1.0.0.dylib
)

FRAMEWORKS_DIR="$STAGING_DIR/${APP_NAME}.app/Contents/Frameworks"
pushd "$FRAMEWORKS_DIR"

for lib in "${LIBS_TO_DEPLOY[@]}"
do
    if [ ! -f "$LIBS_PATH/$lib" ]; then
        echo "Critical: $lib not found in $LIBS_PATH"
        exit 1
    fi

    echo "Processing $lib..."
    cp -v "$LIBS_PATH/$lib" .

    LIBENTRY="${lib%.0.0.dylib}.dylib"
    
    install_name_tool -change $LIBENTRY "@rpath/$LIBENTRY" "../MacOS/$APP_NAME"

    ln -s "$lib" "$LIBENTRY"
done

# FFMPEG

for lib in "${FFMPEG_LIBS[@]}"
do
    if [ ! -f "$LIBS_PATH/$lib" ]; then
        echo "Dependent $lib not found in $LIBS_PATH"
        continue
    fi
    
    echo "Copying $lib..."
    cp -v "$LIBS_PATH/$lib" .
    
    install_name_tool -change $lib "@rpath/$lib" "../MacOS/$APP_NAME"
    # brew fix
    install_name_tool -change "/usr/local/lib/$lib" "@rpath/$lib" "../MacOS/$APP_NAME"

    for depend_lib in "${FFMPEG_LIBS[@]}"
    do
        echo "Fixing dependent $depend_lib..."
        install_name_tool -change "/usr/local/lib/$depend_lib" "@loader_path/$depend_lib" "$lib"
    done
done

popd

RECOVERTY_BUNDLE_PATH="${STAGING_DIR}/${APP_NAME}.app/Contents/MacOS/Recoverty.app"
# fix Recoverty rpath
install_name_tool -add_rpath "@executable_path/../../../../Frameworks" "${RECOVERTY_BUNDLE_PATH}/Contents/MacOS/Recoverty"
# make use of deployed resources of Xpiks
cat << EOF > "${RECOVERTY_BUNDLE_PATH}/Contents/Resources/qt.conf"
[Paths]
Plugins = ../../../PlugIns
Imports = ../../../Resources/qml
Qml2Imports = ../../../Resources/qml
EOF

RESOURCES_DIR="$STAGING_DIR/${APP_NAME}.app/Contents/Resources"

echo "Copying exiftool distribution"
EXIFTOOL_FROM_DIR="${XPIKS_DEPS_DIR}/exiftool"
EXIFTOOL_TO_DIR="$RESOURCES_DIR/exiftool"

if [ ! -d "$EXIFTOOL_TO_DIR" ]; then
    echo "Exiftool directory does not exist. Creating..."
    mkdir "$EXIFTOOL_TO_DIR"
fi

cp "$EXIFTOOL_FROM_DIR/exiftool" "$EXIFTOOL_TO_DIR/"
cp -r "$EXIFTOOL_FROM_DIR/lib" "$EXIFTOOL_TO_DIR/"

# ------------------------------

echo "App dir is packed. Creating DMG..."

# figure out how big our DMG needs to be
#  assumes our contents are at least 1M!
SIZE=`du -sh "${STAGING_DIR}" | sed 's/\([0-9\.]*\)M\(.*\)/\1/'`
SIZE=`echo "${SIZE} + 32.0" | bc | awk '{print int($1+0.5)}'`

if [ $? -ne 0 ]; then
   echo "Error: Cannot compute size of staging dir"
   exit 1
fi

echo "Size is estimated to: $SIZE"

hdiutil create -srcfolder "${STAGING_DIR}" -volname "${VOL_NAME}" -fs HFS+ \
-fsargs "-c c=64,a=16,e=16" -format UDRW -size ${SIZE}m "${DMG_TMP}"

echo "Created DMG: ${DMG_TMP}. Mounting..."

# mount it and save the device
DEVICE=$(hdiutil attach -readwrite -noverify "${DMG_TMP}" | \
         egrep '^/dev/' | sed 1q | awk '{print $1}')

sleep 2

# add a link to the Applications dir
echo "Add link to /Applications"
pushd /Volumes/"${VOL_NAME}"
ln -s /Applications
popd

# add a background image
mkdir /Volumes/"${VOL_NAME}"/.background
cp "${DMG_BACKGROUND_PATH}" /Volumes/"${VOL_NAME}"/.background/

if [ "$BUILD_MODE" == "release" ]; then
    # tell the Finder to resize the window, set the background,
    #  change the icon size, place the icons in the right position, etc.
    echo '
       tell application "Finder"
         tell disk "'${VOL_NAME}'"
               open
               set current view of container window to icon view
               set toolbar visible of container window to false
               set statusbar visible of container window to false
               set the bounds of container window to {480, 300, 980, 570}
               set statusbar visible of container window to false
               set position of every item to {600, 400}
               set viewOptions to the icon view options of container window
               set arrangement of viewOptions to not arranged
               set icon size of viewOptions to 128
               set background picture of viewOptions to file ".background:'${DMG_BACKGROUND_IMG}'"
               set position of item "'${APP_NAME}'.app" of container window to {110, 135}
               set position of item "Applications" of container window to {395, 130}
               set name of item "Applications" to " "
               close
               open
               update without registering applications
               delay 2
         end tell
       end tell
    ' | osascript
fi

sync

# unmount it
hdiutil detach "${DEVICE}"

# now make the final image a compressed disk image
echo "Creating compressed image"
hdiutil convert "${DMG_TMP}" -format UDZO -imagekey zlib-level=9 -o "${DMG_FINAL}"

shasum "${DMG_FINAL}"

# clean up
if [ "$BUILD_MODE" == "debug" ]; then
    rm -rvf "${DMG_TMP}"
fi

echo 'Done.'

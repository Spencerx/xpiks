#!/bin/bash

echo "------------------"
echo "Deploying Xpiks..."
echo "------------------"

pushd src/xpiks-qt/deps
EXIFTOOL_DISTRO="Image-ExifTool-11.30"
EXIFTOOL_ARCHIVE="${EXIFTOOL_DISTRO}.tar.gz"
wget –quiet https://www.sno.phy.queensu.ca/~phil/exiftool/${EXIFTOOL_ARCHIVE}
tar -xf "${EXIFTOOL_ARCHIVE}"
mv ${EXIFTOOL_DISTRO}/* exiftool/
ls -la ./exiftool/
popd

BUILD_MODE=debug BUILD_DIR=src/xpiks-qt DEPLOY_TOOL=macdeployqt VERSION="nightly" APP_NAME="Xpiks" ./scripts/deploy/deploy_mac.sh
mv -v Xpiks-vnightly.dmg Xpiks-Nightly.dmg

echo "Done."

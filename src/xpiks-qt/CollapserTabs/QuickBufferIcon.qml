/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

import QtQuick 2.2
import QtQuick.Layouts 1.1
import "../Constants" 1.0
import "../Components"
import "../StyledControls"
import "../Dialogs"
import "../Common.js" as Common
import "../Constants/UIConfig.js" as UIConfig

Image {
    id: iconWrapper
    anchors.centerIn: parent
    source: isHighlighted ? highlightedIcon : normalIcon

    property string normalIcon: uiColors.t + helpersWrapper.getAssetForTheme("Clipboard_icon_normal.svg", settingsModel.selectedThemeIndex)
    property string highlightedIcon: uiColors.t + helpersWrapper.getAssetForTheme("Clipboard_icon_selected.svg", settingsModel.selectedThemeIndex)

    width: 27
    height: 27
    fillMode: Image.PreserveAspectFit
    sourceSize.width: 27
    sourceSize.height: 27
    cache: true
}

/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

function movePopupInsideComponent(component, window, mouse, old_x, old_y) {
    //            var tmp = root.mapToItem(img,mouse.x,mouse.y);
    var tmp = mapToItem(component, mouse.x, mouse.y);

    var delta_x = tmp.x - old_x;
    var nextX = Math.max(window.x + delta_x, 10);

    if ((nextX + window.width) >= (component.width - 10)) {
        nextX = component.width - 10 - window.width
    }

    window.x = nextX;
    old_x = tmp.x;

    var delta_y = tmp.y - old_y;
    var nextY = Math.max(window.y + delta_y, 10);

    if ((nextY + window.height) >= (component.height - 10)) {
        nextY = component.height - 10 - window.height
    }

    window.y = nextY;
    old_y = tmp.y;

    return [old_x, old_y];
}

function isInComponent(point, component) {
    var result = (point.x >= 0 && point.x <= component.width) &&
            (point.y >= 0 && point.y <= component.height);
    return result;
}

function launchDialog(componentName, directParent, options, functor) {
    console.debug("Opening dialog " + componentName + " ...");

    var component = Qt.createComponent(componentName);
    if (component.status !== Component.Ready) {
        console.warn("Component Error: " + component.errorString());
    } else {
        var instance = component.createObject(directParent, options);        

        if (typeof instance.dialogDestruction !== "undefined") {
            if (typeof directParent.onDialogCreated !== "undefined") {
                directParent.onDialogCreated(componentName)
            }

            if (typeof directParent.onDialogClosed !== "undefined") {
                instance.dialogDestruction.connect(directParent.onDialogClosed);
            }
        }

        if (functor) {
            functor(instance);
        }
    }
}

function safeInsert(textEdit, textToInsert) {
    var originalLength = textToInsert.length;
    textToInsert = textToInsert.replace(/(\r\n|\n|\r)/gm, '');
    var length = textToInsert.length;

    if (length !== originalLength) {
        if (textEdit.selectedText.length > 0) {
            textEdit.remove(textEdit.selectionStart, textEdit.selectionEnd);
        }

        textEdit.insert(textEdit.cursorPosition, textToInsert);
        return true;
    }

    return false;
}

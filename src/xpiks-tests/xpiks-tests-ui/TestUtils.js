function getRandomInt(max) {
    return Math.floor(Math.random() * Math.floor(max));
}

function keyboardEnterSomething(testCase) {
    var keys = [Qt.Key_A, Qt.Key_B, Qt.Key_C, Qt.Key_D, Qt.Key_E, Qt.Key_F, Qt.Key_0, Qt.Key_1]
    var values = ['a', 'b', 'c', 'd', 'e', 'f', '0', '1']
    var count = keys.length + getRandomInt(keys.length)
    var text = "";
    for (var i = 0; i < count; i++) {
        var keyIndex = getRandomInt(keys.length)
        testCase.keyClick(keys[keyIndex])
        text += values[keyIndex]
    }
    return text
}

function clearEdit(editControl) {
    // assigning .text directly breaks binding
    editControl.remove(0, editControl.length)
}

function getDelegateInstanceAt(contentItem, delegateObjectName, index) {
    for(var i = 0; i < contentItem.children.length; ++i) {
        var item = contentItem.children[i];
        // We have to check for the specific objectName we gave our
        // delegates above, since we also get some items that are not
        // our delegates here.
        if (item.objectName == delegateObjectName && item.delegateIndex == index)
            return item;
    }
    return undefined;
}

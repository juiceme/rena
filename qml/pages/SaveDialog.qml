/*
    Copyright 2014 Simo Mattila
    simo.h.mattila@gmail.com

    This file is part of Rena.

    Rena is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    Rena is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Rena.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.0
import Sailfish.Silica 1.0

Dialog {
    id: saveDialog
    allowedOrientations: Orientation.Portrait
    property string name
    property string description
    property string type: qsTr("Running")

    onDone: {
        name = nameField.text;
        description = descriptionField.text;
    }

    Column {
        anchors.fill: parent
        DialogHeader {
            title: "Save track"
            acceptText: "Save"
        }
        TextField {
            id: nameField
            x: Theme.paddingLarge
            width: parent.width - Theme.paddingLarge
            focus: true
            label: qsTr("Name")
            placeholderText: qsTr("Name")
            text: ""
            EnterKey.enabled: true
            EnterKey.iconSource: "image://theme/icon-m-enter-next"
            EnterKey.onClicked: descriptionField.focus = true
        }
        TextArea {
            id: descriptionField
            x: Theme.paddingLarge
            width: parent.width - Theme.paddingLarge
            focus: true
            label: qsTr("Description")
            placeholderText: qsTr("Description")
            text: ""
            EnterKey.enabled: true
            EnterKey.iconSource: "image://theme/icon-m-enter-next"
            EnterKey.onClicked: excerciseType.focus = true
        }
        ComboBox {
            id: excerciseType
            x: Theme.paddingLarge
            width: parent.width - Theme.paddingLarge
            focus: true
            label: qsTr("Type")
            currentIndex: 0

            menu: ContextMenu {
                MenuItem { text: qsTr("Running"); onClicked: saveDialog.type = qsTr("Running") }
                MenuItem { text: qsTr("Hiking"); onClicked: saveDialog.type = qsTr("Hiking") }
                MenuItem { text: qsTr("Cycling"); onClicked: saveDialog.type = qsTr("Cycling") }
                MenuItem { text: qsTr("Skating"); onClicked: saveDialog.type = qsTr("Skating") }
                MenuItem { text: qsTr("Other"); onClicked: saveDialog.type = qsTr("Other") }
            }
            EnterKey.enabled: true
            EnterKey.iconSource: "image://theme/icon-m-enter-accept"
            EnterKey.onClicked: NameField.focus = true
        }
    }
}

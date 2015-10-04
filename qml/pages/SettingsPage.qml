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


Page {
    id: page

    Component.onCompleted: {
        if(settings.updateInterval <= 1000) updateIntervalMenu.currentIndex = 0;
        else if(settings.updateInterval <= 2000) updateIntervalMenu.currentIndex = 1;
        else if(settings.updateInterval <= 5000) updateIntervalMenu.currentIndex = 2;
        else if(settings.updateInterval <= 10000) updateIntervalMenu.currentIndex = 3;
        else if(settings.updateInterval <= 15000) updateIntervalMenu.currentIndex = 4;
        else if(settings.updateInterval <= 30000) updateIntervalMenu.currentIndex = 5;
        else updateIntervalMenu.currentIndex = 6;

        if(settings.dogTrackingInterval == 0) dogTrackingIntervalMenu.currentIndex = 0;
        else if(settings.dogTrackingInterval <= 5000) dogTrackingIntervalMenu.currentIndex = 1;
        else if(settings.dogTrackingInterval <= 10000) dogTrackingIntervalMenu.currentIndex = 2;
        else if(settings.dogTrackingInterval <= 15000) dogTrackingIntervalMenu.currentIndex = 3;
        else if(settings.dogTrackingInterval <= 30000) dogTrackingIntervalMenu.currentIndex = 4;
        else dogTrackingIntervalMenu.currentIndex = 5;

        dogTrackingUrl.text = settings.dogTrackingUrl;
        dogTrackingPassword.text = settings.dogTrackingPassword;
    }

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: childrenRect.height

        Column {
            id: column
            width: page.width
            spacing: Theme.paddingLarge
            PageHeader {
                title: qsTr("Settings")
            }
            ComboBox {
                id: updateIntervalMenu
                label: "Track Point Interval"
                menu: ContextMenu {
                    MenuItem { text: qsTr("1 s (Default)"); onClicked: settings.updateInterval = 1000; }
                    MenuItem { text: qsTr("2 s"); onClicked: settings.updateInterval = 2000; }
                    MenuItem { text: qsTr("5 s"); onClicked: settings.updateInterval = 5000; }
                    MenuItem { text: qsTr("10 s"); onClicked: settings.updateInterval = 10000; }
                    MenuItem { text: qsTr("15 s"); onClicked: settings.updateInterval = 15000; }
                    MenuItem { text: qsTr("30 s"); onClicked: settings.updateInterval = 30000; }
                    MenuItem { text: qsTr("1 minute"); onClicked: settings.updateInterval = 60000; }
                }
            }
            ComboBox {
                id: dogTrackingIntervalMenu
                label: "Dog Tracking Interval"
                description: qsTr('Connects to external service to track your dog')
                menu: ContextMenu {
                    MenuItem { text: qsTr("off"); onClicked: settings.dogTrackingInterval = 0; }
                    MenuItem { text: qsTr("5 s"); onClicked: settings.dogTrackingInterval = 5000; }
                    MenuItem { text: qsTr("10 s"); onClicked: settings.dogTrackingInterval = 10000; }
                    MenuItem { text: qsTr("15 s"); onClicked: settings.dogTrackingInterval = 15000; }
                    MenuItem { text: qsTr("30 s"); onClicked: settings.dogTrackingInterval = 30000; }
                    MenuItem { text: qsTr("1 minute"); onClicked: settings.dogTrackingInterval = 60000; }
                }
            }
            TextField {
                id: dogTrackingUrl
                visible: settings.dogTrackingInterval != 0
                x: Theme.paddingLarge
                width: parent.width - Theme.paddingLarge
                label: qsTr("Dog Tracking URL")
                placeholderText: qsTr("https://my.own.site/my-runaway-dog/")
                EnterKey.enabled: true
                EnterKey.iconSource: "image://theme/icon-m-enter-next"
                onFocusChanged: {
                    if(!focus) {
                        settings.dogTrackingUrl = dogTrackingUrl.text;
                    }
                }
            }
            TextField {
                id: dogTrackingPassword
                visible: settings.dogTrackingInterval != 0
                echoMode: TextInput.PasswordEchoOnEdit
                x: Theme.paddingLarge
                width: parent.width - Theme.paddingLarge
                label: qsTr("Dog Tracking Password")
                placeholderText: qsTr("**********")
                EnterKey.enabled: true
                EnterKey.iconSource: "image://theme/icon-m-enter-next"
                onFocusChanged: {
                    if(!focus) {
                        settings.dogTrackingPassword = dogTrackingPassword.text;
                    }
                }
            }
        }
    }
}

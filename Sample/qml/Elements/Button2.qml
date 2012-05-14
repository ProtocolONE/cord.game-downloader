/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates.
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/

import QtQuick 1.0

Rectangle {
    id: button2Id
    width: buttonText2.width + 30
    height: 28

    border { color: "white"; width: 1 }

    property string buttonText: "OK"
    property Gradient buttonActiveGradient: Gradient{ GradientStop { position: 1; color: "#030303" } GradientStop { position: 0; color: "#4a4b49" } }
    property Gradient nullGradient: Gradient {}

    gradient: buttonActiveGradient;
    signal buttonClicked();

    Text{
        id: buttonText2
        text: buttonText
        anchors.verticalCenterOffset: -1
        anchors.horizontalCenter: parent.horizontalCenter

        anchors.verticalCenter: parent.verticalCenter
        font.family: "Tahoma"
        font.bold: false
        font.pixelSize: 14
        font.weight: "Light"
        opacity: 1
        smooth: true

        color: "white"
    }

    MouseArea{
        anchors.fill: parent
        hoverEnabled: true
        onEntered: {
            button2Id.color = "#ff9800"
            button2Id.gradient = nullGradient
        }
        onExited: {
            button2Id.color = "transparent"
            button2Id.gradient = buttonActiveGradient
        }
        onClicked: buttonClicked();
    }
}

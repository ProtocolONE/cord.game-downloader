// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1
import Tulip 1.0
import "../js/PopupHelper.js" as PopupHelper
import "../Elements" as Elements

Elements.PopupItemBase {
    signal closeButtonClicked();
    signal playClicked();

    property string message: ""
    property string buttonCaption: "Играть"
    property variant gameItem

    width: 209
    height: 171

    Item {
        anchors.fill: parent

        Rectangle {
            id: backgroundRectangle
            color: "#FF9900"
            border { color: "#33FFFFFF"; width: 1 }
            anchors { fill: parent; leftMargin: 1; topMargin: -1; rightMargin: 1; bottomMargin: 1 }
        }

        Image {
            id: closeButtonImage

            anchors { right: parent.right; top: parent.top; rightMargin: 9; topMargin: 9 }
            visible: containsMouse || closeButtonImageMouser.containsMouse || puttonPlayMouser.containsMouse
            source: installPath + "images/closeButton.png"
            opacity: 0.5

            NumberAnimation {
                id: closeButtonDownAnimation;
                running: false;
                target: closeButtonImage;
                property: "opacity";
                from: 0.9;
                to: 0.5;
                duration: 225
            }

            NumberAnimation {
                id: closeButtonUpAnimation;
                running: false;
                target: closeButtonImage;
                property: "opacity";
                from: 0.5;
                to: 0.9;
                duration: 225
            }

            Elements.CursorMouseArea {
                id: closeButtonImageMouser

                anchors.fill: parent
                hoverEnabled: true
                onClicked: {
                    closeButtonClicked();
                    shadowDestroy();
                }

                onEntered: closeButtonUpAnimation.start()
                onExited: closeButtonDownAnimation.start()
            }
        }

        Image {
            anchors { top: parent.top; topMargin: 23; horizontalCenter: parent.horizontalCenter }
            source: installPath + gameItem.imageLogoSmall
        }

        Text {
            id: messageTextItem
            anchors { fill: parent; leftMargin: 5; rightMargin: 5; topMargin: 85; bottomMargin: 50 }
            wrapMode: TextEdit.WordWrap
            font.pixelSize: 12
            text: message
            color: "#673102"
            horizontalAlignment: Text.AlignHCenter
        }

        Rectangle {
            id: buttonRectangle
            property color normalColor: "#d68200"
            property color hoverColor: "#cd7a00"
            anchors { left: parent.left; right: parent.right; bottom: parent.bottom }
            anchors { leftMargin: 2; rightMargin: 1; bottomMargin: 1 }

            height: 43
            color: puttonPlayMouser.containsMouse ? hoverColor : normalColor

            Elements.TextH2 {
                anchors { verticalCenter: parent.verticalCenter; horizontalCenter: parent.horizontalCenter }
                text: buttonCaption
                color: "#ffffff"
            }

            Elements.CursorMouseArea {
                id: puttonPlayMouser

                anchors.fill: parent
                hoverEnabled: true

                onClicked: {
                    playClicked();
                    shadowDestroy();
                }
            }
        }
    }

    state: "Green"
    states: [
        State {
            name: "Orange"
            PropertyChanges { target: backgroundRectangle; color: "#FF9900" }
            PropertyChanges { target: buttonRectangle; normalColor: "#d68200" }
            PropertyChanges { target: buttonRectangle; hoverColor: "#CD7A00" }
            PropertyChanges { target: messageTextItem; color: "#673102" }
        },
        State {
            name: "Green"
            PropertyChanges { target: backgroundRectangle; color: "#019934" }
            PropertyChanges { target: buttonRectangle; normalColor: "#00822a" }
            PropertyChanges { target: buttonRectangle; hoverColor: "#007922" }
            PropertyChanges { target: messageTextItem; color: "#003300" }
        }
    ]
}

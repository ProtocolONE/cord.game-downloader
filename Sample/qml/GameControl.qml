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
import "Elements" as Elements


Item {
    id: gameControl

    property string gameId:  "0000011"
    property string statusText:  "status text "
    property int progressValue: 0

  	width: parent.width
	  height: 50

    Text {
        x: gameControl.x
        y: gameControl.y - 10
        text: gameControl.gameId
    }

    Elements.Button2 {
        x: gameControl.x
        y: gameControl.y
        buttonText: "start"
        onButtonClicked: {
            //console.log("start " + gameId);
            mainWindow.startTorrent(gameId);
        }
    }

    Elements.Button2 {
        x: gameControl.x + 60
        y: gameControl.y
        buttonText: "stop"
        onButtonClicked:{
            mainWindow.stopTorrent(gameId);
        }
    }

    Elements.Button2 {
        x: gameControl.x + 120
        y: gameControl.y
        buttonText: "replace"
        onButtonClicked:{
            mainWindow.restartTorrent(gameId);
        }
    }

    Elements.Button2 {
        x: gameControl.x + 200
        y: gameControl.y
        buttonText: "recheck"
        onButtonClicked:{
            mainWindow.recheckTorrent(gameId);
        }
    }

    Elements.ProgressBar3{
        id: progressBar
        x: gameControl.x + 280
        y: gameControl.y
        width: 600
        color: "red"
        value: progressValue
    }

    Text {
        x: gameControl.x
        y: gameControl.y + 30
        text:       statusText
    }

    Elements.Input {
        id: archiveInput
        x: gameControl.x
        y: gameControl.y + 45
        editDefaultText: "archive directory"
        height: 20
        width: 400
    }

    Elements.Input {
        id: extracionInput
        x: gameControl.x
        y: gameControl.y + 75
        editDefaultText: "game directory"
        height: 20
        width: 400
    }

    Elements.Button2 {
        x: gameControl.x + 450
        y: gameControl.y + 45
        buttonText: "submit"
        onButtonClicked:{
            mainWindow.changeDirectory(gameId, archiveInput.editText, extracionInput.editText);
        }
    }

    Connections {
        target: mainWindow

        onServiceProgressChanged:{
            if (id != gameControl.gameId)
                return;
            gameControl.progressValue = progress;
        }

        onServiceStatusChanged: {
            if (id != gameControl.gameId)
                return;
            gameControl.statusText = status;
        }

		onServiceProgressChanged2: {
            if (id != gameControl.gameId)
                return;
            gameControl.progressValue = progress;
		}
    }


}


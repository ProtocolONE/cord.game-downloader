/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (В©) 2011 - 2012, Syncopate Limited and/or affiliates.
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/

import QtQuick 1.0
import "Elements" as Elements

Item {
//	property int x: 0
//	property int y: 0
	id: container;
  property int inputHeight: 30
  property int inputWidth: 50
	property int buttonWidth: 50

//	property alias text: portInput.text

	signal submitClicked(string text)

	Rectangle {
		x: 0
		y: 0
		width: container.inputWidth
		height: container.inputHeight
		border.color: "red"
		border.width: 1
		Elements.Input2 {
			id: portInput
			x: 1
			y: 1
			width: container.inputWidth - 2
			height: container.inputHeight - 2
			text: "123"
			//editDefaultText: "Port"
		}
  }

	Elements.Button2 {
		x: container.inputWidth + 10
		y: 0
		buttonText: "submit"
		width: buttonWidth
		height: inputHeight
		onButtonClicked: {
			submitClicked(portInput.text);
			//console.log("changePort " + portInput.editText);
			//mainWindow.startTorrent(gameId);
			//mainWindow.setPort(portInput.text);
			//mainWindow.torrentChangePort();
		}
  }

  
}

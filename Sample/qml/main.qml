// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1
import "./Elements" as Elements
import "." as Blocks
Rectangle {
     id: container
     width: 1000; height: 800

	Rectangle {
		x: 10
		y: 10
		width: 71
		height: 21
		border.color: "red"
		border.width: 1
		Elements.Input2 {
			id: portInput
			x: 1
			y: 1
			width: 70
			height: 20
			text: mainWindow.port
			//editDefaultText: "Port"
		}
	}

	Elements.Button2 {
		x: 900
		y: 5
		buttonText: "quit"
		onButtonClicked: {
			mainWindow.shutdown();
		}
  }

	
	Elements.Button2 {
		x: 90
		y: 5
		buttonText: "changePort"
		onButtonClicked: {
			//console.log("changePort " + portInput.editText);
			//mainWindow.startTorrent(gameId);
			mainWindow.setPort(portInput.text);
			mainWindow.torrentChangePort();
		}
  }

	Text { text: 'upload'; x: 250; y: 5}
	Blocks.InputWithButton {
		x: 300
		y: 5
		inputHeight: 20
		inputWidth: 130
		onSubmitClicked: {
			console.log('input submit ' + text);
			mainWindow.setUploadRateLimit(text);
		}
	}

	Text { text: 'download'; x: 250; y: 35}
	Blocks.InputWithButton {
		x: 300
		y: 35
		inputHeight: 20
		inputWidth: 130
		onSubmitClicked: {
			console.log('input submit2 ' + text);
			mainWindow.setDownloadRateLimit(text);
		}
	}


	Blocks.GameControl {
		x: 5
		y: 40
		gameId: "300002010000000000"
//		progressValue: mainWindow.progress1
//		statusText: mainWindow.statusText1
	}

	Blocks.GameControl {
		x: 5
		y: 70+30
		gameId: "300003010000000000"
//		progressValue: mainWindow.progress2
//		statusText: mainWindow.statusText2
	}

	Blocks.GameControl {
		x: 5
		y: 100+60
		gameId: "300004010000000000"
//		progressValue: mainWindow.progress2
//		statusText: mainWindow.statusText2
	}

	Blocks.GameControl {
		x: 5
		y: 130+90
		gameId: "300005010000000000"
//		progressValue: mainWindow.progress2
//		statusText: mainWindow.statusText2
	}

	Blocks.GameControl {
		x: 5
		y: 160+120
		gameId: "300006010000000000"
//		progressValue: mainWindow.progress2
//		statusText: mainWindow.statusText2
	}
 }

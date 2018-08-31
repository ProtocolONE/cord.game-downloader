import QtQuick 1.1

Rectangle {
	
	x: 0
	y: 400
	width: progressPercent * 800 / 100
	height: 50
        color: "red"


	id: baseRect
	property int progressPercent

	border.width: 2
	border.color: "black"
/*
     Rectangle {
	x: 1
	y: 400
	width: baseRect.width * 100 / progressPercent
	height: 50
        color: "red"
     }
*/
}

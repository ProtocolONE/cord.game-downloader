/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates.
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/

import QtQuick 1.1
import "../js/Core.js" as Core
Item {
    id: item

    property ListModel model
    property Component delegate
    property int count: 0
    property int spacing: 10

    Column {
        spacing: item.spacing

        Row {
            id: topRow
            spacing: item.spacing
        }

        Row {
            id: bottomRow
            spacing: item.spacing
        }
    }

    function getAnimationPause(serviceId) {
        var delay  = {
            "300003010000000000" : 0,
            "300009010000000000" : 75,
            "300002010000000000" : 150,
            "300007010000000000" : 225,
            "300005010000000000" : 300,
            "300006010000000000" : 375,
            "300011010000000000" : 450,
        }

        return delay[serviceId] ? delay[serviceId] : 0;
    }

    // В функции по текущей модели создаются элементы и заполняется контейнер.
    // Удаления элементов пока не предусмотренно, так как по факту коллекция в runtime
    // не меняется. При необходимости можно будет дописать.
    function fill() {
        if (!model || !delegate) {
            console.log("FlowView error: model or delegate is empty")
            return;
        }

        if (model.count < 1) {
            return;
        }

        var listItem, index;

        var topRowList = [ "300003010000000000" , "300009010000000000" , "300002010000000000" ];
        var bottomRowList = [ "300007010000000000" , "300005010000000000", "300006010000000000", "300011010000000000"];

        topRowList.forEach(function(e) {
            listItem = delegate.createObject(topRow, {model: Core.serviceItemByServiceId(e)});
            listItem.animationPause = getAnimationPause(e);
        })

        bottomRowList.forEach(function(e) {
            listItem = delegate.createObject(bottomRow, {model: Core.serviceItemByServiceId(e)});
            listItem.animationPause = getAnimationPause(e);
        })

        count = model.count;
    }

    Component.onCompleted: {
        fill();
    }
}

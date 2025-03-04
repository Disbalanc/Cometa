// MapComponent.qml
import "qrc:/ui/DataDisplay/Map/map"
import "qrc:/ui/DataDisplay/Map/forms"

import QtQuick 2.12
import QtLocation 5.12
import QtPositioning 5.12
import App.Logging 1.0

Map {
    id: mainMap
    gesture.enabled: true
    gesture.acceptedGestures: MapGestureArea.PanGesture |
                            MapGestureArea.FlickGesture |
                            MapGestureArea.PinchGesture |
                            MapGestureArea.RotationGesture

    property alias minimapLoader: minimapLoader

    // Слайдеры управления
    MapSliders {
        mapSource: mainMap
        edge: Qt.LeftEdge
        expanded: true
        z: mainMap.z + 3
    }

    // Миникарта
    Loader {
        id: minimapLoader
        active: false
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: 10
        sourceComponent: Component {
            Map {
                width: 200
                height: 200
                plugin: mainMap.plugin
                center: mainMap.center
                zoomLevel: mainMap.zoomLevel - 2
            }
        }
        onLoaded: {
            logger.log(Logger.Debug, "Миникарта успешно загружена");
                        item.minimapReady.connect(function() {
                            logger.log(Logger.Info, "Миникарта полностью инициализирована");
                        });
        }
    }

    Component.onCompleted: {
        logger.log(Logger.Info, "Основная карта инициализирована");
        center = QtPositioning.coordinate(52, 34)
        zoomLevel = 5
    }
}

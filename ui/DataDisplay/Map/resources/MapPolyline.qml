import QtQuick 2.5
import QtLocation 5.6
import QtPositioning 5.5

Item {
    id: mapContainer
    width: 800
    height: 600

    Map {
        id: map
        anchors.fill: parent
        plugin: Plugin {
            name: "osm"
        }
        center: QtPositioning.coordinate(52, 34)
        zoomLevel: 5

        // Полилиния для отображения пути
        MapPolyline {
            id: pathPolyline
            line.width: 2
            line.color: "blue"
            path: []
        }

        function loadMarkers(markers) {
            var path = [];
            for (var i = 0; i < markers.length; i++) {
                var marker = markers[i];
                path.push(QtPositioning.coordinate(marker.latitude, marker.longitude));
            }
            pathPolyline.path = path; // Устанавливаем путь для полилинии
            console.log("Путь загружен с", path.length, "точками.");
        }

        Component.onCompleted: {
            // Пример добавления точки
            addMarker(52, 34, 10, 180);
            addMarker(60, 20, 0, 0);
        }
    }
}

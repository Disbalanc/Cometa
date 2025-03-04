import QtQuick 2.12
import QtLocation 5.12
import QtPositioning 5.12

Item {
    id: root
    width: 800
    height: 600

    function logCoordinate(coord) {
        return "(" + coord.latitude.toFixed(6) + ", " + coord.longitude.toFixed(6) + ")"
    }

    property string mapProvider: "osm"
    property string mapType: "Street"
    property color routeColor: "blue"
    property bool showMarkers: true
    property int pointDensity: 1
    property var coordinates: []

    Plugin {
        id: mapPlugin
        name: root.mapProvider
        PluginParameter { name: "osm.mapping.host"; value: "https://tile.openstreetmap.org/" }
    }

    Map {
        id: map
        anchors.fill: parent
        plugin: mapPlugin
        activeMapType: supportedMapTypes[findMapTypeIndex()]
        copyrightsVisible: false

        MapPolyline {
            id: route
            line.width: 2
            line.color: root.routeColor
        }

        function updateRoute() {
            console.log("=== Начало обновления маршрута ===")
            console.log("Всего координат:", coordinates.length)
            console.log("Плотность точек:", pointDensity)

            var path = []
            for (var i = 0; i < coordinates.length; i += pointDensity) {
                var coord = coordinates[i]
                path.push(QtPositioning.coordinate(coord.latitude, coord.longitude))

                // Логирование каждой 10-й точки
                if (i % (10 * pointDensity) === 0) {
                    console.log("Добавлена точка #" + i + ": " + logCoordinate(path[path.length-1]))
                }
            }

            console.log("Итого точек в маршруте:", path.length)
            route.path = path
            console.log("Полилиния обновлена. Первая точка:", logCoordinate(path[0]),
                      "Последняя точка:", logCoordinate(path[path.length-1]))

            fitViewToRoute()
        }

        function fitViewToRoute() {
            if (route.path.length === 0) {
                console.warn("Пустой маршрут - пропуск масштабирования");
                return;
            }

            console.log("=== Масштабирование карты ===");

            // Инициализация минимальных и максимальных значений
            var minLat = route.path[0].latitude;
            var maxLat = route.path[0].latitude;
            var minLon = route.path[0].longitude;
            var maxLon = route.path[0].longitude;

            // Находим границы маршрута
            for (var i = 1; i < route.path.length; i++) {
                var coord = route.path[i];
                minLat = Math.min(minLat, coord.latitude);
                maxLat = Math.max(maxLat, coord.latitude);
                minLon = Math.min(minLon, coord.longitude);
                maxLon = Math.max(maxLon, coord.longitude);
            }

            // Добавляем отступы (10% от размера маршрута)
            var latPadding = (maxLat - minLat) * 0.1;
            var lonPadding = (maxLon - minLon) * 0.1;

            // Рассчитываем новые границы с отступами
            var paddedMinLat = Math.max(minLat - latPadding, -90);
            var paddedMaxLat = Math.min(maxLat + latPadding, 90);
            var paddedMinLon = Math.max(minLon - lonPadding, -180);
            var paddedMaxLon = Math.min(maxLon + lonPadding, 180);

            // Создаем прямоугольник с отступами
            var topLeft = QtPositioning.coordinate(paddedMaxLat, paddedMinLon);
            var bottomRight = QtPositioning.coordinate(paddedMinLat, paddedMaxLon);
            var paddedRect = QtPositioning.rectangle(topLeft, bottomRight);

            // Рассчитываем центр
            var centerLat = (paddedMaxLat + paddedMinLat) / 2;
            var centerLon = (paddedMaxLon + paddedMinLon) / 2;

            // Рассчитываем уровень зума
            var latDiff = paddedMaxLat - paddedMinLat;
            var lonDiff = paddedMaxLon - paddedMinLon;
            var maxDiff = Math.max(latDiff, lonDiff);
            var zoomLevel = Math.log2(360 / maxDiff) - 1.0; // Настроенная формула

            // Ограничиваем уровень зума
            zoomLevel = Math.min(Math.max(zoomLevel, 4), 18);

            // Устанавливаем параметры карты
            map.center = QtPositioning.coordinate(centerLat, centerLon);
            map.zoomLevel = zoomLevel;

            console.log("Новые границы:",
                "Широта:", paddedMinLat.toFixed(6), "-", paddedMaxLat.toFixed(6),
                "Долгота:", paddedMinLon.toFixed(6), "-", paddedMaxLon.toFixed(6));
            console.log("Уровень зума:", zoomLevel.toFixed(2));
        }

        function findMapTypeIndex() {
            for(var i = 0; i < supportedMapTypes.length; i++) {
                if(supportedMapTypes[i].name === root.mapType) return i
            }
            return 0
        }
    }

    function addMarkers() {
        console.log("=== Добавление маркеров ===")
        console.log("Удаление старых маркеров...")

        var markers = map.mapItems.filter(item => item.objectName === "marker")
        console.log("Найдено старых маркеров:", markers.length)

        markers.forEach(item => item.destroy())
        console.log("Старые маркеры удалены")

        console.log("Создание новых маркеров...")
        var createdMarkers = 0

        for(var i = 0; i < coordinates.length; i += pointDensity) {
            var component = Qt.createComponent("Marker.qml")
            if(component.status === Component.Ready) {
                var marker = component.createObject(map, {
                    objectName: "marker",
                    coordinate: QtPositioning.coordinate(
                        coordinates[i].latitude,
                        coordinates[i].longitude
                    ),
                    speed: coordinates[i].speed,
                    course: coordinates[i].course,
                    visible: showMarkers
                })

                if (marker) {
                    map.addMapItem(marker)
                    createdMarkers++

                    // Логирование каждой 10-й точки
                    if (createdMarkers % 10 === 0) {
                        console.log("Создан маркер #" + createdMarkers + ":",
                                  logCoordinate(marker.coordinate))
                    }
                }
            } else {
                console.error("Ошибка загрузки компонента Marker.qml:", component.errorString())
            }
        }

        console.log("Всего создано маркеров:", createdMarkers)
    }

    onCoordinatesChanged: {
        console.log("Координаты изменены. Новое количество:", coordinates.length)
        if(coordinates.length > 0) {
            console.log("Первая координата:", logCoordinate(QtPositioning.coordinate(
                coordinates[0].latitude,
                coordinates[0].longitude
            )))
            map.updateRoute()
            if(showMarkers) addMarkers()
        }
    }

    Component.onCompleted: {
        console.log("Компонент карты инициализирован")
        if(coordinates.length > 0) {
            console.log("Начальная загрузка координат")
            console.log("Инициализация карты с размером", width, height)
            map.updateRoute()
            if(showMarkers) addMarkers()
        }
    }
}

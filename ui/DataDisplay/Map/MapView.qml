import "qrc:/ui/DataDisplay/Map/forms"
import "qrc:/ui/DataDisplay/Map/map"
import "qrc:/ui/DataDisplay/Map/resources"

import QtQuick 2.12
import QtQuick.Controls 2.12
import QtLocation 5.12
import QtPositioning 5.12
import QtQuick.Dialogs 1.3
import App.Logging 1.0

Item {
    id: mapContainer
    width: 800
    height: 600

    property bool markersVisible: true
    property var markerCoordinates: []
    property var polyline: null
    property color polylineColor: "blue"
    property real polylineWidth: 2
    property alias currentMap: mapLoader.item

    // Панель управления
    Column {
        id: controlPanel
        spacing: 10
        anchors {
            top: parent.top
            left: parent.left
            margins: 10
        }

        ComboBox {
            id: providerCombo
            width: 200
            model: []
            onCurrentIndexChanged: {
                    if (currentIndex >= 0) {
                        initMap(model[currentIndex]);
                    }
                }
        }

        ComboBox {
            id: mapTypeCombo
            width: 200
            model: []
            onActivated: {
                if(currentMap && currentMap.supportedMapTypes) {
                    currentMap.activeMapType = currentMap.supportedMapTypes[index]
                }
            }
        }

        Row {
            spacing: 10
            Button {
                text: "Загрузить данные"
                onClicked: loadFlightDialog.open()
            }
            CheckBox {
                text: "Показать маркеры"
                checked: markersVisible
                onCheckedChanged: {
                        markersVisible = checked;
                        updateMarkersVisibility();
                    }
            }
            Button {
                text: "Очистить карту"
                onClicked: clearMap()
            }
            Button {
                id: miniMapBtn
                text: "Показать миникарту"
                onClicked: toggleMiniMap()
            }
        }
    }

    // Загрузчик основной карты
    Loader {
        id: mapLoader
        anchors {
            top: controlPanel.bottom
            left: parent.left
            right: parent.right
            bottom: parent.bottom
            margins: 10
        }
        // Обработчик сигнала loaded объявляем здесь
                onLoaded: {
                    try {
                        logger.log(Logger.Debug, "Основная карта загружена, инициализация плагина...");
                        currentMap.plugin = Qt.createQmlObject(
                            'import QtLocation 5.12; Plugin { name: "' + currentProvider + '"}',
                            mapContainer
                        )
                        updateMapTypes()
                        currentMap.center = QtPositioning.coordinate(52, 34)
                        currentMap.zoomLevel = 5
                        logger.log(Logger.Info, "Карта успешно инициализирована с провайдером: " + currentProvider);
                    } catch(e) {
                        logger.log(Logger.Error, "Ошибка инициализации карты: " + e.message);
                    }
                }
                onStatusChanged: {
                            if (status === Loader.Error) {
                                logger.log(Logger.Error, "Ошибка загрузки карты: ");
                            }
                        }
    }

    // Диалог загрузки
    LoadFlightDialog {
        id: loadFlightDialog
        onAccepted: loadMarkers(data)
    }

    property string currentProvider: "osm"

    function updateMarkersVisibility() {
        if(currentMap) {
            var items = currentMap.mapItems;
            for (var i = 0; i < items.length; i++) {
                if (items[i].objectName === "marker") {
                    items[i].visible = markersVisible;
                }
            }
        }
    }

    function initMap(provider) {
        logger.log(Logger.Info, "Инициализация карты с провайдером: " + provider);
        currentProvider = provider;
        mapLoader.sourceComponent = undefined; // Сброс предыдущего компонента
        mapLoader.sourceComponent = Qt.createComponent("map/MapComponent.qml");

        // Обработчик изменения статуса загрузки
        mapLoader.onStatusChanged.connect(function() {
            if (mapLoader.status === Loader.Ready) {
                logger.log(Logger.Info, "Инициализация карты с провайдером: " + provider);
            }
        });
    }

    function updateMapTypes() {
        if(currentMap && currentMap.supportedMapTypes) {
            var types = []
            for(var i = 0; i < currentMap.supportedMapTypes.length; i++) {
                types.push(currentMap.supportedMapTypes[i].name)
            }
            mapTypeCombo.model = types
        }
    }

    function addMarker(latitude, longitude, speed, course, id, date, time, altitude) {
        if(!currentMap) {
            logger.log(Logger.Error, "Попытка добавить маркер: карта не инициализирована");
            return;
        }

        var component = Qt.createComponent("resources/Marker.qml");
        logger.log(Logger.Debug, "Создание компонента маркера, статус: " + component.status);

        if (component.status === Component.Ready) {
            var marker = component.createObject(currentMap, {
                objectName: "marker", // Добавить это свойство
                coordinate: QtPositioning.coordinate(latitude, longitude),
                speed: speed,
                course: course,
                markerId: id,
                date: date,
                time: time,
                altitude: altitude,
                visible: markersVisible
            });
            if (marker) {
                currentMap.addMapItem(marker);
                markerCoordinates.push(marker.coordinate);
                logger.log(Logger.Info, "Маркер добавлен: " + marker.coordinate);
                            } else {
                                logger.log(Logger.Error, "Ошибка создания объекта маркера");
            }
        } else {
            logger.log(Logger.Error, "Ошибка загрузки компонента маркера: " + component.errorString());
        }
    }

    function loadMarkers(markers) {
        if (!markers || !Array.isArray(markers)) {
            logger.log(Logger.Warning, "Попытка загрузки пустых маркеров");
                        return;
                }
        logger.log(Logger.Info, "Начало загрузки " + markers.length + " маркеров");
                clearMap();
        for(var i = 0; i < markers.length; i++) {
            var m = markers[i]
            addMarker(m.latitude, m.longitude, m.speed, m.course,
                     m.id, m.date, m.time, m.altitude)
        }
        drawPolyline()
    }

    function toggleMiniMap() {
        if(currentMap && currentMap.minimapLoader) {
            var newState = !currentMap.minimapLoader.active;
            currentMap.minimapLoader.active = newState;
            miniMapBtn.text = newState ? "Скрыть миникарту" : "Показать миникарту";
            logger.log(Logger.Info, "Переключение миникарты: " + (newState ? "вкл" : "выкл"));
        } else {
            logger.log(Logger.Error, "Не удалось переключить миникарту: компоненты не инициализированы");
        }
    }

    function clearMap() {
        if(currentMap) {
            // Удаляем маркеры
            var items = currentMap.mapItems;
            for (var i = items.length - 1; i >= 0; i--) {
                if (items[i].objectName === "marker") {
                    items[i].destroy();
                }
            }
            markerCoordinates = [];

            // Удаляем старую полилинию
            if(polyline) {
                polyline.destroy();
                polyline = null;
            }
        }
    }

    function drawPolyline() {
        if(!currentMap) return

        if(polyline) polyline.destroy()
        polyline = Qt.createQmlObject(
            'import QtLocation 5.12; MapPolyline {' +
            'line.color: "' + polylineColor + '";' +
            'line.width: ' + polylineWidth + ';' +
            'path: ' + JSON.stringify(markerCoordinates) + '}',
            currentMap
        )
        currentMap.addMapItem(polyline)
    }

    Component.onCompleted: {
        var plugins = []
        logger.log(Logger.Info, "Инициализация компонента карты");
        var basePlugin = Qt.createQmlObject('import QtLocation 5.12; Plugin {}', mapContainer)
        logger.log(Logger.Debug, "Доступные провайдеры карт: " + basePlugin.availableServiceProviders);
        for(var i = 0; i < basePlugin.availableServiceProviders.length; i++) {
            try {
                var p = Qt.createQmlObject(
                    'import QtLocation 5.12; Plugin {name: "' +
                    basePlugin.availableServiceProviders[i] + '"}',
                    mapContainer
                )
                if(p.supportsMapping()) plugins.push(basePlugin.availableServiceProviders[i])
            } catch(e) {
                console.error("Ошибка загрузки плагина:", e)
            }
        }
        providerCombo.model = plugins
        if(plugins.length > 0) initMap(plugins[0])
    }
    Connections {
            target: dbManager
            onDataLoaded: function(data) {
                logger.log(Logger.Info, "Получены данные от dbManager, количество записей: " + (data ? data.length : 0));
                if (data && data.length > 0) {
                    loadMarkers(data); // Теперь data содержит список объектов NavigationDataMap
                }
            }
        }
}

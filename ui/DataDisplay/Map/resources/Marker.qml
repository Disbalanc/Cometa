import QtQuick 2.5
import QtLocation 5.6
import QtPositioning 5.5

MapQuickItem {
    id: marker
    property bool isMarker: true
    property real latitude: 0
    property real longitude: 0
    property real speed: 0
    property real course: 0
    property int id: 0
    property string date: ""
    property string time: ""
    property real altitude: 0

    coordinate: QtPositioning.coordinate(latitude, longitude)

    sourceItem: Item {
        width: 32
        height: 32

        Image {
            id: image
            source: course === 0 ? "qrc:/ui/DataDisplay/Map/resources/location.png" : "qrc:/ui/DataDisplay/Map/resources/arrow.png"
            width: 32
            height: 32
            rotation: course === 0 ? 0 : course
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
        }

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true

            onEntered: {
                idText.visible = true
                speedText.visible = true
                courseText.visible = true
                altitudeText.visible = true
                dateTimeText.visible = true
            }

            onExited: {
                idText.visible = false
                speedText.visible = false
                courseText.visible = false
                altitudeText.visible = false
                dateTimeText.visible = false
            }
        }

        Text {
            id: idText
            text: id
            font.pixelSize: 12
            color: "black"
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: image.bottom
            visible: false
        }

        Text {
            id: speedText
            text: "S: " + speed.toFixed(1) + " m/s"
            font.pixelSize: 12
            color: "black"
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: idText.bottom
            visible: false
        }

        Text {
            id: courseText
            text: "C: " + course.toFixed(1) + "°"
            font.pixelSize: 12
            color: "black"
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: speedText.bottom
            visible: false
        }

        Text {
            id: altitudeText
            text: "A: " + altitude.toFixed(1) + " m"
            font.pixelSize: 12
            color: "black"
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: courseText.bottom
            visible: false
        }

        Text {
            id: dateTimeText
            text: date + " " + time
            font.pixelSize: 12
            color: "black"
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: altitudeText.bottom
            visible: false
        }
    }
}

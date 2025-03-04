import QtQuick 2.0
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.5

Dialog {
    id: loadFlightDialog
    title: qsTr("Загрузка данных полёта")
    modal: true
    width: 600
    height: 500
    anchors.centerIn: parent

    property string selectedFlightId: ""
    property string filterField: ""
    property string filterValue: ""
    property string sortField: ""
    property string sortOrder: "ASC"

    background: Rectangle {
        color: "#f5f5f5"
        radius: 8
        border.color: "#cccccc"
    }

    header: Label {
        text: title
        font.bold: true
        font.pixelSize: 18
        padding: 15
        color: "#2c3e50"
        background: Rectangle {
            color: "#ecf0f1"
            radius: 8
        }
    }

    ColumnLayout {
        spacing: 15
        anchors.fill: parent
        anchors.margins: 15

        // Блок выбора полёта
        GroupBox {
            title: qsTr("Выбор полёта")
            Layout.fillWidth: true
            background: Rectangle {
                color: "transparent"
                border.color: "#bdc3c7"
            }

            ComboBox {
                id: flightComboBox
                model: dbManager.getAllFlightsMap()
                Layout.fillWidth: true

                font.pixelSize: 14

                onCurrentTextChanged: selectedFlightId = currentText
            }
        }

        // Блок фильтрации
        GroupBox {
            title: qsTr("Параметры фильтрации")
            Layout.fillWidth: true
            background: Rectangle {
                color: "transparent"
                border.color: "#bdc3c7"
            }

            ColumnLayout {
                spacing: 10
                width: parent.width
                Text {
                    text: qsTr("Выберите поле для фильтрации")
                }
                ComboBox {
                    id: filterComboBox
                    model: ["ID", "Time", "Date", "Latitude", "Longitude", "Altitude", "Speed", "Course", "IsValid", "TimeStamp"]

                    Layout.fillWidth: true
                    font.pixelSize: 14
                    background: Rectangle {
                        color: "white"
                        border.color: "#bdc3c7"
                        radius: 4
                    }
                    onCurrentTextChanged: filterField = currentText
                }

                TextField {
                    id: filterLineEdit
                    placeholderText: qsTr("Введите значение фильтра")
                    Layout.fillWidth: true
                    font.pixelSize: 14
                    background: Rectangle {
                        color: "white"
                        border.color: "#bdc3c7"
                        radius: 4
                    }
                    onTextChanged: filterValue = text
                }
            }
        }

        // Блок сортировки
        GroupBox {
            title: qsTr("Параметры сортировки")
            Layout.fillWidth: true
            background: Rectangle {
                color: "transparent"
                border.color: "#bdc3c7"
            }

            RowLayout {
                spacing: 10
                width: parent.width
                Text {
                    text: qsTr("Сортировать по")
                }
                ComboBox {
                    id: sortComboBox
                    model: ["ID", "Time", "Date", "Latitude", "Longitude", "Altitude", "Speed", "Course", "IsValid", "TimeStamp"]

                    Layout.fillWidth: true
                    font.pixelSize: 14
                    background: Rectangle {
                        color: "white"
                        border.color: "#bdc3c7"
                        radius: 4
                    }
                    onCurrentTextChanged: sortField = currentText
                }

                ComboBox {
                    id: orderComboBox
                    model: ["По возрастанию", "По убыванию"]
                    currentIndex: 0
                    Layout.preferredWidth: 150
                    font.pixelSize: 14
                    background: Rectangle {
                        color: "white"
                        border.color: "#bdc3c7"
                        radius: 4
                    }
                    onCurrentTextChanged: sortOrder = currentText === "По возрастанию" ? "ASC" : "DESC"
                }
            }
        }

        // Кнопки
        RowLayout {
            spacing: 15
            Layout.alignment: Qt.AlignRight

            Button {
                text: qsTr("Отмена")
                flat: true
                onClicked: reject()
                contentItem: Text {
                    text: parent.text
                    color: "#e74c3c"
                    font.pixelSize: 14
                    horizontalAlignment: Text.AlignHCenter
                }
                background: Rectangle {
                    color: "transparent"
                }
            }

            Button {
                text: qsTr("Загрузить")
                contentItem: Text {
                    text: parent.text
                    color: "white"
                    font.pixelSize: 14
                    horizontalAlignment: Text.AlignHCenter
                }
                background: Rectangle {
                    color: "#3498db"
                    radius: 4
                }
                onClicked: accept()
            }
        }
    }

    onAccepted: {
        console.log(flightComboBox.count)
        dbManager.getNavigationDataFilterValidMap(
            filterField,
            filterValue,
            sortField,
            sortOrder,
            selectedFlightId
        )
    }
}

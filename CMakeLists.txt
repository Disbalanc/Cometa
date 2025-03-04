cmake_minimum_required(VERSION 3.5)

# Укажите имя проекта
project(Cometa VERSION 1.0 LANGUAGES CXX)

# Укажите, что мы используем Qt
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)


# Укажите необходимые модули Qt
# Укажите необходимые модули Qt
find_package(Qt5 REQUIRED COMPONENTS
    Core
    Gui
    3DCore
    3DRender
    3DInput
    3DExtras
    Network
    Qml
    SerialPort
    Sql
    Location
    DataVisualization
    Charts
    Positioning
    Concurrent
    PrintSupport
    Quick
    QuickControls2
    Widgets
    QuickWidgets
    LinguistTools
)
find_package(QT NAMES Qt6 Qt5)

# Для работы с SSL
find_package(OpenSSL 1.1.1 REQUIRED)

# Укажите стандарт C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Укажите пути к заголовочным файлам
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib/qcustomplot)

# Укажите исходные файлы
file(GLOB PROJECT_SOURCES
    main.cpp
    data/Resources/Cometa_ru_RU.ts
    data/Resources/Cometa_en_EN.ts
    data/Resources/upkbLogo.png
    lib/qcustomplot/qcustomplot.cpp
    ui/DataDisplay/mapwidget.cpp
    data/Managers/connectionmanager.cpp
    data/Managers/datamanager.cpp
    data/Class/formatnavigationdata.cpp
    ui/DataDisplay/setupchartstab.cpp
    ui/DataDisplay/setupgraphtab.cpp
    ui/DataDisplay/setuptabletab.cpp
    ui/MainWindow/mainwindow.cpp
    ui/DataDisplay/datadisplaywindow.cpp
    data/Managers/databasemanager.cpp
    data/Class/ethernetclient.cpp
    data/Class/logger.cpp
    data/Class/parsernmea.cpp
    data/Class/udpsocket.cpp
    ui/Settings/settings.cpp
    ui/DataDisplay/reporttab.cpp
    data/Class/aianalyzer.cpp
    data/Class/navigationdata.cpp
    ui/DataDisplay/tableconfigdialog.cpp
    main.cpp
)

# Укажите заголовочные файлы
file(GLOB PROJECT_HEADERS
    lib/qcustomplot/qcustomplot.h
    ui/DataDisplay/mapwidget.h
    data/Managers/connectionmanager.h
    data/Managers/datamanager.h
    data/Class/formatnavigationdata.h
    ui/DataDisplay/setupchartstab.h
    ui/DataDisplay/setupgraphtab.h
    ui/DataDisplay/setuptabletab.h
    ui/MainWindow/mainwindow.h
    ui/DataDisplay/datadisplaywindow.h
    data/Managers/databasemanager.h
    data/Class/ethernetclient.h
    data/Class/logger.h
    data/Class/parsernmea.h
    data/Class/udpsocket.h
    ui/Settings/settings.h
    ui/DataDisplay/reporttab.h
    data/Class/aianalyzer.h
    data/Class/NavigationData.h
    ui/DataDisplay/tableconfigdialog.h
)

# Укажите файлы форм
file(GLOB PROJECT_FORMS
    ui/DataDisplay/datadisplaywindow.ui
    ui/MainWindow/mainwindow.ui
    ui/Settings/settings.ui
)

# Укажите QML файлы и другие ресурсы как обычные файлы
set(QML_FILES
    ui/DataDisplay/Map/MapView.qml
    ui/DataDisplay/Map/resources/Marker.qml
    ui/DataDisplay/Map/resources/location.png
    ui/DataDisplay/Map/resources/arrow.png
    ui/DataDisplay/Map/resources/MapPolyline.qml
    ui/DataDisplay/Map/map/MapComponent.qml
    ui/DataDisplay/Map/map/MapSliders.qml
    ui/DataDisplay/Map/forms/LoadFlightDialog.qml
    ui/DataDisplay/Map/ReportMap.qml
)

set(RESOURCES_FILES
    resources.qrc
)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES} ${PROJECT_HEADERS} ${PROJECT_FORMS} ${RESOURCES_FILES} ${QML_FILES})

# Настройка свойств SSL
target_compile_definitions(${PROJECT_NAME} PRIVATE
    QT_NETWORK_LIB
    QT_SSL
)

# Укажите пути к заголовочным файлам для компилятора
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}lib/qcustomplot
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/DataDisplay
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/MainWindow
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/Settings
    ${CMAKE_CURRENT_SOURCE_DIR}/data/Class
    ${CMAKE_CURRENT_SOURCE_DIR}/data/Managers
)

# Свяжите библиотеки Qt с вашим проектом
target_link_libraries(${PROJECT_NAME} Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Gui Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::3DCore Qt${QT_VERSION_MAJOR}::3DRender Qt${QT_VERSION_MAJOR}::3DInput Qt${QT_VERSION_MAJOR}::3DExtras
                      Qt${QT_VERSION_MAJOR}::Network Qt${QT_VERSION_MAJOR}::SerialPort Qt${QT_VERSION_MAJOR}::Sql Qt${QT_VERSION_MAJOR}::Location Qt${QT_VERSION_MAJOR}::DataVisualization
                      Qt${QT_VERSION_MAJOR}::Charts Qt${QT_VERSION_MAJOR}::Positioning Qt${QT_VERSION_MAJOR}::Concurrent Qt${QT_VERSION_MAJOR}::PrintSupport
                      Qt${QT_VERSION_MAJOR}::Quick Qt${QT_VERSION_MAJOR}::QuickControls2 OpenSSL::SSL Qt5::QuickWidgets
                      OpenSSL::Crypto)

                  # Экспорт свойства supportsSsl в QML
                  target_compile_definitions(${PROJECT_NAME} PRIVATE
                      -DQT_QML_DEBUG
                      -DQT_NETWORK_LIB
                      -DQT_SSL
                  )

                  # Для передачи SSL статуса в QML
                  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
                      target_compile_definitions(${PROJECT_NAME} PRIVATE -DQT_QML_DEBUG)
                  endif()

                  # Дополнительные настройки для QML
                  set_target_properties(${PROJECT_NAME} PROPERTIES
                      MACOSX_BUNDLE_GUI_IDENTIFIER com.example.Cometa
                      MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
                      MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
                      MACOSX_BUNDLE TRUE
                      WIN32_EXECUTABLE TRUE
                  )

# Google Test
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${GTEST_LIBRARIES} pthread)

# Включение предупреждений о устаревших API
add_definitions(-DQT_DEPRECATED_WARNINGS)

# Установка
install(TARGETS ${PROJECT_NAME} DESTINATION bin)

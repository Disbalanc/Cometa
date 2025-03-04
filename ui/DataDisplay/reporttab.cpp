#include "reporttab.h"

#include <QApplication>
#include <QColorDialog>
#include <QThread>
#include <QDebug>
#include <QTimer>
#include <QJsonArray>
#include <QJsonDocument>
#include <QMessageBox>
#include <QJsonObject>
#include <QLineSeries>
#include <QtMath>
#include <QPrinter>
#include <QInputDialog>
#include <QProcess>
#include <QTemporaryFile>

ReportTab::ReportTab(DatabaseManager *dbManager,Logger *logger, QWidget *parent)
    : QWidget(parent), dbManager(dbManager), mapLineColor(Qt::blue),
    mapDensity(1), showMarkers(true), mapProvider("osm"),m_logger(logger)
{
    m_logger->log(Logger::Info, "Инициализация ReportTab...");
    qRegisterMetaType<NavigationData>("NavigationData");
    qRegisterMetaType<QList<NavigationData>>("QList<NavigationData>");
    aiAnalyzer = new AIAnalyzer(this); // Добавить эту строку
    setupUI();

    // Подключение сигналов AIAnalyzer
    connect(aiAnalyzer, &AIAnalyzer::analysisComplete, this, &ReportTab::onAIAnalysisComplete);
    connect(aiAnalyzer, &AIAnalyzer::errorOccurred, this, &ReportTab::onAIError);
}

void ReportTab::setupUI() {
    // Основной макет
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(15);

    // Поле для отчета (левая часть)
    reportTextEdit = new QTextEdit(this);
    reportTextEdit->setReadOnly(false);
    reportTextEdit->setPlaceholderText("Сгенерированный отчет будет отображен здесь...");
    reportTextEdit->setMinimumWidth(450);
    mainLayout->addWidget(reportTextEdit);

    // Правая панель для управления
    QVBoxLayout *rightPanelLayout = new QVBoxLayout();
    rightPanelLayout->setSpacing(15);
    rightPanelLayout->setAlignment(Qt::AlignTop);

    // Секция: Основные параметры
    QGroupBox *mainParamsGroup = new QGroupBox("Основные параметры", this);
    QVBoxLayout *mainParamsLayout = new QVBoxLayout(mainParamsGroup);

    // Выбор полета
    QHBoxLayout *flightLayout = new QHBoxLayout();
    flightLayout->addWidget(new QLabel("Выбор полета:", this));
    flightComboBox = new QComboBox(this);
    flightComboBox->addItems(dbManager->getAllFlights());
    flightLayout->addWidget(flightComboBox);
    mainParamsLayout->addLayout(flightLayout);

    // Фильтрация
    QGroupBox *filterGroup = new QGroupBox("Фильтрация данных", this);
    QFormLayout *filterLayout = new QFormLayout(filterGroup);
    filterComboBox = new QComboBox(this);
    filterComboBox->addItems({"ID", "Time", "Date", "Latitude", "Longitude",
                              "Altitude", "Speed", "Course", "IsValid", "TimeStamp"});
    filterLineEdit = new QLineEdit(this);
    filterLineEdit->setPlaceholderText("Значение для фильтрации");
    filterLayout->addRow("Поле фильтрации:", filterComboBox);
    filterLayout->addRow("Значение:", filterLineEdit);
    mainParamsLayout->addWidget(filterGroup);

    // Сортировка
    QGroupBox *sortGroup = new QGroupBox("Сортировка данных", this);
    QFormLayout *sortLayout = new QFormLayout(sortGroup);
    sortComboBox = new QComboBox(this);
    sortComboBox->addItems({"ID", "Time", "Date", "Latitude", "Longitude",
                            "Altitude", "Speed", "Course", "IsValid", "TimeStamp"});
    orderComboBox = new QComboBox(this);
    orderComboBox->addItems({"По возрастанию", "По убыванию"});
    sortLayout->addRow("Поле сортировки:", sortComboBox);
    sortLayout->addRow("Порядок:", orderComboBox);
    mainParamsLayout->addWidget(sortGroup);

    rightPanelLayout->addWidget(mainParamsGroup);

    // Секция: Конструктор отчета
    QGroupBox *reportConstructorGroup = new QGroupBox("Конструктор отчета", this);
    QVBoxLayout *constructorLayout = new QVBoxLayout(reportConstructorGroup);

    // Выбор блоков
    QGroupBox *blockSelectionGroup = new QGroupBox("Типы блоков", this);
    QGridLayout *blockLayout = new QGridLayout(blockSelectionGroup);

    // Создаем группу для эксклюзивного выбора
    QButtonGroup *blockTypeGroup = new QButtonGroup(this);

    // Создаем радио-кнопки
    tableBlockRadio = new QRadioButton("Таблица", this);
    chartBlockRadio = new QRadioButton("График", this);
    mapBlockRadio = new QRadioButton("Карта", this);
    textBlockRadio = new QRadioButton("Текст", this);
    //aiTextBlockRadio = new QRadioButton("AI текст", this);
    graphBlockRadio = new QRadioButton("3D График", this);

    // Добавляем кнопки в группу
    blockTypeGroup->addButton(tableBlockRadio);
    blockTypeGroup->addButton(chartBlockRadio);
    blockTypeGroup->addButton(mapBlockRadio);
    blockTypeGroup->addButton(textBlockRadio);
    //blockTypeGroup->addButton(aiTextBlockRadio);
    blockTypeGroup->addButton(graphBlockRadio);


    // Располагаем кнопки в сетке
    blockLayout->addWidget(tableBlockRadio, 0, 0);
    blockLayout->addWidget(mapBlockRadio, 0, 1);
    blockLayout->addWidget(graphBlockRadio, 1, 0);
    blockLayout->addWidget(chartBlockRadio, 1, 1);
    blockLayout->addWidget(textBlockRadio, 2, 0);
    //blockLayout->addWidget(aiTextBlockRadio, 2, 1);

    constructorLayout->addWidget(blockSelectionGroup);

    // Настройки графиков
    chartSettingsGroup = new QGroupBox("Настройки графиков", this);
    QFormLayout *chartLayout = new QFormLayout(chartSettingsGroup);
    chartTypeSelector = new QComboBox(this);
    chartTypeSelector->addItems({"Линейный", "Spline", "Точечный", "Столбчатый", "Круговой"});
    xAxisSelector = new QComboBox(this);
    xAxisSelector->addItems({"Время","Широта", "Долгота", "Высота", "Скорость", "Курс"});

    yAxisList = new QListWidget(this);
    yAxisList->setSelectionMode(QAbstractItemView::MultiSelection);
    QStringList yOptions = {"Скорость", "Курс","Высота", "Широта", "Долгота"};
    yAxisList->addItems(yOptions);

    chartLayout->addRow("Тип графика:", chartTypeSelector);
    chartLayout->addRow("Ось X:", xAxisSelector);
    chartLayout->addRow("Ось Y:", yAxisList);

    QHBoxLayout *colorLayout = new QHBoxLayout();
    colorButton = new QPushButton("Цвет линии", this);
    colorLayout->addWidget(new QLabel("Цвет:"));
    colorLayout->addWidget(colorButton);

    densitySpinBox = new QSpinBox(this);
    densitySpinBox->setRange(1, 100);
    densitySpinBox->setValue(1);
    densitySpinBox->setToolTip("Плотность точек (1 = все точки)");

    chartLayout->addRow(colorLayout);
    chartLayout->addRow("Плотность точек:", densitySpinBox);
    constructorLayout->addWidget(chartSettingsGroup);
    chartSettingsGroup->setVisible(false); // Скрыть по умолчанию

    // Настройки 3D графика
    scatter3DSettingsGroup = new QGroupBox("Настройки 3D графика", this);
    QFormLayout *scatter3DLayout = new QFormLayout(scatter3DSettingsGroup);

    // Цвет точек
    QPushButton *color3DButton = new QPushButton("Выбрать цвет", this);
    color3DButton->setStyleSheet(QString("background-color: %1;").arg(scatterColor.name()));
    scatter3DLayout->addRow("Цвет точек:", color3DButton);

    // Размер точек
    QSpinBox *size3DSpin = new QSpinBox(this);
    size3DSpin->setRange(1, 100);
    size3DSpin->setValue(scatterPointSize);
    scatter3DLayout->addRow("Размер точек:", size3DSpin);

    // Плотность точек
    QSpinBox *density3DSpin = new QSpinBox(this);
    density3DSpin->setRange(1, 100);
    density3DSpin->setValue(scatterDensity);
    scatter3DLayout->addRow("Плотность точек:", density3DSpin);

    // Проекция камеры
    cameraProjectionCombo = new QComboBox(this);
    cameraProjectionCombo->addItems({"Перспектива", "Изометрия"});
    scatter3DLayout->addRow("Проекция:", cameraProjectionCombo);

    constructorLayout->addWidget(scatter3DSettingsGroup);
    scatter3DSettingsGroup->setVisible(false);

    // Настройки таблицы
    tableSettingsGroup = new QGroupBox("Настройки таблицы", this);
    QVBoxLayout *tableLayout = new QVBoxLayout(tableSettingsGroup);
    QStringList columns = {"ID", "Время (UTC)", "Дата", "Широта (°)", "Долгота (°)",
                           "Высота (м)", "Скорость (м/с)", "Курс (°)", "Действителен"};
    for(const QString &col : columns) {
        QCheckBox *cb = new QCheckBox(col, this);
        cb->setChecked(true);  // Активируем чекбоксы по умолчанию
        tableLayout->addWidget(cb);
    }
    constructorLayout->addWidget(tableSettingsGroup);
    tableSettingsGroup->setVisible(false); // Скрыть по умолчанию

    rightPanelLayout->addWidget(reportConstructorGroup);

    // В секции конструктора отчета после добавления tableSettingsGroup
    textSettingsGroup = new QGroupBox("Настройки текста", this);
    QVBoxLayout *textLayout = new QVBoxLayout(textSettingsGroup);
    textEdit = new QTextEdit(this);
    textEdit->setPlaceholderText("Введите текст для блока...");
    textLayout->addWidget(textEdit);
    constructorLayout->addWidget(textSettingsGroup);
    textSettingsGroup->setVisible(false);

    // Настройки карты
    mapSettingsGroup = new QGroupBox("Настройки карты", this);
    QFormLayout *mapLayout = new QFormLayout(mapSettingsGroup);

    // Выбор провайдера
    QComboBox *providerCombo = new QComboBox(this);
    providerCombo->addItems({"osm", "esri", "here"});
    mapLayout->addRow("Провайдер:", providerCombo);

    // Выбор типа карты
    QComboBox *mapTypeCombo = new QComboBox(this);
    mapTypeCombo->addItems({"Street", "Satellite", "Hybrid"});
    mapLayout->addRow("Тип карты:", mapTypeCombo);

    // Цвет линии
    QPushButton *colorMapButton = new QPushButton("Цвет линии", this);
    colorMapButton->setStyleSheet(QString("background-color: %1;").arg(mapLineColor.name()));
    mapLayout->addRow("Цвет линии:", colorMapButton);

    // Плотность точек
    QSpinBox *densityMapSpin = new QSpinBox(this);
    densityMapSpin->setRange(1, 100);
    densityMapSpin->setValue(mapDensity);
    mapLayout->addRow("Плотность точек:", densityMapSpin);

    // Показ маркеров
    QCheckBox *markersCheck = new QCheckBox("Показывать маркеры", this);
    markersCheck->setChecked(showMarkers);
    mapLayout->addRow(markersCheck);

    constructorLayout->addWidget(mapSettingsGroup);
    mapSettingsGroup->setVisible(false);

    // Кнопки управления
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    addBlockButton = new QPushButton("Добавить блок", this);
    generateButton = new QPushButton("Сгенерировать отчет", this);
    saveButton = new QPushButton("Сохранить отчет", this);
    buttonLayout->addWidget(addBlockButton);
    buttonLayout->addWidget(generateButton);
    buttonLayout->addWidget(saveButton);
    rightPanelLayout->addLayout(buttonLayout);

    colorButton->setStyleSheet(QString("background-color: %1;").arg(chartColor.name()));

    // Сигналы
    connect(chartBlockRadio, &QRadioButton::toggled,
            chartSettingsGroup, &QGroupBox::setVisible);
    connect(tableBlockRadio, &QRadioButton::toggled,
            tableSettingsGroup, &QGroupBox::setVisible);

    // Коннекты для кнопок
    connect(addBlockButton, &QPushButton::clicked,
            this, &ReportTab::onAddBlock);
    connect(generateButton, &QPushButton::clicked,
            this, &ReportTab::onGenerateReport);
    connect(saveButton, &QPushButton::clicked,
            this, &ReportTab::onSaveReport);
    connect(textBlockRadio, &QRadioButton::toggled,
            textSettingsGroup, &QGroupBox::setVisible);
    connect(colorButton, &QPushButton::clicked, this, &ReportTab::selectChartColor);
    connect(densitySpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            [this](int value){ pointDensity = value; });
    connect(graphBlockRadio, &QRadioButton::toggled,
            scatter3DSettingsGroup, &QGroupBox ::setVisible);
    connect(color3DButton, &QPushButton::clicked, this, [this, color3DButton]() {
        QColor color = QColorDialog::getColor(scatterColor, this);
        if (color.isValid()) {
            scatterColor = color;
            color3DButton->setStyleSheet(QString("background-color: %1;").arg(scatterColor.name()));
        }
    });
    connect(size3DSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int size) {
        scatterPointSize = size;
    });
    connect(density3DSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int density) {
        scatterDensity = density;
    });
    connect(cameraProjectionCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
        // Обработка изменения проекции камеры
    });
    connect(mapBlockRadio, &QRadioButton::toggled, mapSettingsGroup, &QGroupBox::setVisible);
    connect(colorMapButton, &QPushButton::clicked, this, [this, colorMapButton]() {
        QColor color = QColorDialog::getColor(mapLineColor, this);
        if(color.isValid()) {
            mapLineColor = color;
            colorMapButton->setStyleSheet(QString("background-color: %1;").arg(color.name()));
        }
    });
    connect(densityMapSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, [this](int density){ mapDensity = density; });
    connect(markersCheck, &QCheckBox::toggled, this, [this](bool checked){ showMarkers = checked; });
    connect(providerCombo, &QComboBox::currentTextChanged, this, [this](const QString &text){ mapProvider = text; });
    connect(mapTypeCombo, &QComboBox::currentTextChanged, this, [this](const QString &text){ mapType = text; });

    mainLayout->addLayout(rightPanelLayout);
    m_logger->log(Logger::Info, "UI ReportTab успешно настроен");
}

void ReportTab::onAddBlock() {
    m_logger->log(Logger::Info, "Попытка добавить новый блок в отчет");
    if (fligth_name != flightComboBox->currentText() || fligth_name.isNull()) {
        fligth_name = flightComboBox->currentText();
        m_logger->log(Logger::Debug,
                      QString("Изменение выбранного полета на: %1").arg(fligth_name));
        generateReportHeader();
    }

    try {
        QTextCursor cursor(reportTextEdit->textCursor());
    cursor.movePosition(QTextCursor::End);

    if (tableBlockRadio->isChecked()) {
        generateTableBlock(cursor);
    } else if (chartBlockRadio->isChecked()) {
        generateChartBlock(cursor);
    } else if (graphBlockRadio->isChecked()) {
        generate3DChartBlock(cursor);
    } else if (mapBlockRadio->isChecked()) {
        generateMapBlock(cursor);
    } else if (textBlockRadio->isChecked()) {
        generateTextBlock(cursor);
    }
    m_logger->log(Logger::Info, "Блок успешно добавлен в отчет");
    } catch(const std::exception& e) {
            m_logger->log(Logger::Error,
                          QString("Ошибка при добавлении блока: %1").arg(e.what()));
        QMessageBox::critical(this, "Ошибка", "Не удалось добавить блок в отчет");
    }
}

void ReportTab::generateAITextBlock(QTextCursor &cursor) {
    QList<NavigationData> data = getFilteredData();
    if(data.isEmpty()) return;

    // Сериализация данных в JSON
    QJsonArray jsonData;
    foreach(const auto &item, data) {
        auto d = item.deserialize();
        jsonData.append(QJsonObject{
            {"time", d.gnzda.time.toString("hh:mm:ss")},
            {"latitude", d.gnrmc.latitude},
            {"longitude", d.gnrmc.longitude},
            {"speed", d.gnrmc.speed},
            {"altitude", d.gngga.altitude},
            {"isValid", d.gnrmc.isValid}
        });
    }

    aiAnalyzer->analyzeDataBlack(QJsonDocument(jsonData).toJson());
}

void ReportTab::generateMapBlock(QTextCursor &cursor) {
    m_logger->log(Logger::Debug, "Начало генерации блока карты");
    QList<NavigationData> data = getFilteredData();
    if(data.isEmpty()){
        m_logger->log(Logger::Warning, "Пустые данные для генерации карты");
        return;
    }
    try {
        // Создаем QQuickView
        QQuickView *view = new QQuickView();
        view->setSource(QUrl("qrc:/ui/DataDisplay/Map/ReportMap.qml"));
        if(view->status() == QQuickView::Error) {
            m_logger->log(Logger::Error,
                          QString("Ошибка загрузки QML карты: %1")
                              .arg(view->errors().first().description()));
            return;
        }
        // Подготавливаем данные для QML
        QVariantList coords;
        for(const auto &item : data) {
            auto d = item.deserialize();
            if(d.gnrmc.isValid &&
                !qFuzzyIsNull(d.gnrmc.latitude) &&
                !qFuzzyIsNull(d.gnrmc.longitude)) {
                coords.append(QVariantMap{
                    {"latitude", d.gnrmc.latitude},
                    {"longitude", d.gnrmc.longitude},
                    {"speed", d.gnrmc.speed},
                    {"course", d.gnrmc.course}
                });
            }
        }
        if(coords.isEmpty()) return;

        // Настраиваем параметры карты
        QQuickItem *root = view->rootObject();
        root->setProperty("mapProvider", mapProvider);
        root->setProperty("mapType", mapType);
        root->setProperty("routeColor", mapLineColor);
        root->setProperty("showMarkers", showMarkers);
        root->setProperty("pointDensity", mapDensity);
        root->setProperty("coordinates", coords);

        // Создаем контейнер и принудительно обновляем
        QWidget *container = QWidget::createWindowContainer(view, this); // Указываем родителя
        container->setMinimumSize(800, 600);
        container->show();
        container->setMinimumSize(800, 600);
        container->setVisible(true);
        container->show();
        container->update();

        // Ожидание полной инициализации
        QEventLoop loop;
        QObject::connect(view, &QQuickView::statusChanged, [&](QQuickView::Status status) {
            if(status == QQuickView::Ready) loop.quit();
        });
        QTimer::singleShot(2000, &loop, &QEventLoop::quit); // Увеличенный таймаут
        loop.exec();

        // Дополнительная обработка событий
        QCoreApplication::processEvents();
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);

        // Захват изображения
        QPixmap pixmap = QPixmap::fromImage(view->grabWindow());
        if(!pixmap.isNull()) {
            insertPixmapToDocument(pixmap, cursor);
        } else {
            qDebug() << "Ошибка: не удалось получить изображение карты";
        }

        // Очистка ресурсов
        container->deleteLater();
        view->deleteLater();

        QString caption = QString("<div style='text-align: center; margin: 10px 0; font-style: italic; color: #7f8c8d;'><br>"
                                  "Рисунок %1: Карта полета</div>")
                              .arg(++figureCounter);
        cursor.insertHtml(caption);
        m_logger->log(Logger::Info, "Блок карты успешно сгенерирован");
    } catch(const std::exception& e) {
        m_logger->log(Logger::Error,
                      QString("Ошибка генерации блока карты: %1").arg(e.what()));
    }
}

QPixmap ReportTab::renderMapToPixmap() {
    if(mapView) {
        QImage image = mapView->grabWindow();
        return QPixmap::fromImage(image);
    }
    return QPixmap();
}

void ReportTab::onGenerateReport() {
    m_logger->log(Logger::Info, "Начало генерации полного отчета");
    figureCounter = 0;
    reportTextEdit->clear();
    try {
    generateReport();

    QTextCursor cursor = reportTextEdit->textCursor();
    cursor.movePosition(QTextCursor::End);
    cursor.insertHtml(QString(
                          "<div style='text-align: right; color: #7f8c8d; margin: 20px 0; font-size: 0.9em;'>"
                          "Отчет сгенерирован: %1</div>"
                          ).arg(QDateTime::currentDateTime().toString("dd.MM.yyyy HH:mm")));
    m_logger->log(Logger::Info, "Отчет успешно сгенерирован");
    } catch(const std::exception& e) {
        if(m_logger) {
            m_logger->log(Logger::Error,
                          QString("Ошибка генерации отчета: %1").arg(e.what()));
        }
        QMessageBox::critical(this, "Ошибка", "Не удалось сгенерировать отчет");
    }
}

void ReportTab::generateReport() {
    QTextCursor cursor = reportTextEdit->textCursor();
    QList<NavigationData> data = getFilteredData();
    if(data.isEmpty()){
        m_logger->log(Logger::Warning, "Пустые данные для генерации отчета");
        return;
    }
    m_logger->log(Logger::Debug,
                  QString("Генерация отчета для %1 записей").arg(data.size()));
    try {
    // Шапка отчета
    generateReportHeader();

    // Основные показатели
    cursor.insertHtml("<h2 style='color: #2c3e50; border-bottom: 2px solid #3498db; padding-bottom: 10px; margin-top: 20px;'>"
                      "Основные показатели полета</h2>");

    if(!data.isEmpty()) {
        auto [minAlt, maxAlt, avgSpeed, maxSpeed, avgCourse,

              flightDuration, minLat, maxLat, minLon, maxLon] = calculateFlightMetrics(data);
        cursor.insertHtml(QString(
            "<div style='margin: 20px 0; padding: 20px; background: #f8f9fa; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1);'><br>"
            "<h3 style='color: #34495e; margin-top: 0;'>Статистика полета</h3>"
            "<div style='display: grid; grid-template-columns: repeat(3, 1fr); gap: 15px;'>"
            "<div style='padding: 15px; background: white; border-radius: 6px; border: 1px solid #eee;'>"
            "<div style='font-size: 0.9em; color: #7f8c8d;'>Максимальная высота</div>"
            "<div style='font-size: 1.2em; font-weight: bold; color: #2c3e50;'>%1 м</div>"
            "</div>"
            "<div style='padding: 15px; background: white; border-radius: 6px; border: 1px solid #eee;'>"
            "<div style='font-size: 0.9em; color: #7f8c8d;'>Минимальная высота</div>"
            "<div style='font-size: 1.2em; font-weight: bold; color: #2c3e50;'>%2 м</div>"
            "</div>"
                              "<div style='padding: 15px; background: white; border-radius: 6px; border: 1px solid #eee;'>"
                              "<div style='font-size: 0.9em; color: #7f8c8d;'>Минимальная широта</div>"
                              "<div style='font-size: 1.2em; font-weight: bold; color: #2c3e50;'>%1°</div>"
                              "</div>"
                              "<div style='padding: 15px; background: white; border-radius: 6px; border: 1px solid #eee;'>"
                              "<div style='font-size: 0.9em; color: #7f8c8d;'>Максимальная широта</div>"
                              "<div style='font-size: 1.2em; font-weight: bold; color: #2c3e50;'>%2°</div>"
                              "</div>"
                              "<div style='padding: 15px; background: white; border-radius: 6px; border: 1px solid #eee;'>"
                              "<div style='font-size: 0.9em; color: #7f8c8d;'>Минимальная долгота</div>"
                              "<div style='font-size: 1.2em; font-weight: bold; color: #2c3e50;'>%3°</div>"
                              "</div>"
                              "<div style='padding: 15px; background: white; border-radius: 6px; border: 1px solid #eee;'>"
                              "<div style='font-size: 0.9em; color: #7f8c8d;'>Максимальная долгота</div>"
                              "<div style='font-size: 1.2em; font-weight: bold; color: #2c3e50;'>%4°</div>"
                              "</div>"
            "<div style='padding: 15px; background: white; border-radius: 6px; border: 1px solid #eee;'>"
            "<div style='font-size: 0.9em; color: #7f8c8d;'>Средняя скорость</div>"
            "<div style='font-size: 1.2em; font-weight: bold; color: #2c3e50;'>%3 м/с</div>"
            "</div>"
            "<div style='padding: 15px; background: white; border-radius: 6px; border: 1px solid #eee;'>"
            "<div style='font-size: 0.9em; color: #7f8c8d;'>Максимальная скорость</div>"
            "<div style='font-size: 1.2em; font-weight: bold; color: #2c3e50;'>%4 м/с</div>"
            "</div>"
            "<div style='padding: 15px; background: white; border-radius: 6px; border: 1px solid #eee;'>"
            "<div style='font-size: 0.9em; color: #7f8c8d;'>Средний курс</div>"
            "<div style='font-size: 1.2em; font-weight: bold; color: #2c3e50;'>%5°</div>"
            "</div>"
            "<div style='padding: 15px; background: white; border-radius: 6px; border: 1px solid #eee;'>"
            "<div style='font-size: 0.9em; color: #7f8c8d;'>Длительность полета</div>"
            "<div style='font-size: 1.2em; font-weight: bold; color: #2c3e50;'>%6</div>"
            "</div>"
            "</div></div>"
        ).arg(maxAlt, 0, 'f', 1)
         .arg(minAlt, 0, 'f', 1)
         .arg(avgSpeed, 0, 'f', 1)
         .arg(maxSpeed, 0, 'f', 1)
         .arg(avgCourse, 0, 'f', 1)
         .arg(formatDuration(flightDuration))
         .arg(minLat, 0, 'f', 6)
                              .arg(maxLat, 0, 'f', 6)
                              .arg(minLon, 0, 'f', 6)
                              .arg(maxLon, 0, 'f', 6));
    }

    // Блок с картой
    cursor.insertHtml("<h2 style='color: #2c3e50; border-bottom: 2px solid #3498db; padding-bottom: 10px; margin-top: 30px;'><br>"
                     "Географический маршрут</h2>");
    generateMapBlock(cursor);

    // Описание карты
    cursor.insertHtml(
        "<div style='margin: 20px 0; padding: 15px; background: #f8f9fa; border-radius: 6px;'><br>"
        "<h3 style='color: #34495e; margin-top: 0;'>Описание маршрута</h3>"
        "<p style='line-height: 1.6; color: #666;'>"
        "На представленной карте отображен полный маршрут полета. "
        "Синяя линия обозначает траекторию движения, маркеры показывают ключевые точки. "
        "Масштаб карты позволяет детально рассмотреть особенности маршрута."
        "</p></div>"
    );

    // Блок с графиками
    cursor.insertHtml("<h2 style='color: #2c3e50; border-bottom: 2px solid #3498db; padding-bottom: 10px; margin-top: 30px;'><br>"
                     "Динамика параметров</h2>");
    // Автоматический выбор всех параметров для оси Y
    for(int i = 0; i < yAxisList->count()-2; ++i) {
        yAxisList->item(i)->setSelected(true);
    }
    generateChartBlock(cursor);

    // Описание графика
    cursor.insertHtml(
        "<div style='margin: 20px 0; padding: 15px; background: #f8f9fa; border-radius: 6px;'><br>"
        "<h3 style='color: #34495e; margin-top: 0;'>Анализ графиков</h3>"
        "<p style='line-height: 1.6; color: #666;'>"
        "График отображает изменение параметров полета во времени. "
        "Использована линейная интерполяция данных. По оси X - временная шкала, "
        "по оси Y - значения выбранных параметров. Цветовые обозначения соответствуют легенде."
        "</p></div>"
    );

    // График широты
    cursor.insertHtml("<h2 style='color: #2c3e50; border-bottom: 2px solid #3498db; padding-bottom: 10px; margin-top: 30px;'><br>"
                      "Динамика широты</h2>");
    for(int i = 0; i < yAxisList->count()-2; ++i) {
        yAxisList->item(i)->setSelected(false);
    }
    yAxisList->item(3)->setSelected(true);
    generateChartBlock(cursor);
    yAxisList->item(3)->setSelected(false);

    cursor.insertHtml(
        "<br><div style='margin: 20px 0; padding: 15px; background: #f8f9fa; border-radius: 6px;'>"
        "<p style='line-height: 1.6; color: #666;'>"
        "График отображает изменение широты в течение полета. "
        "Положительные значения соответствуют северной широте, отрицательные - южной."
        "</p></div>"
        );

    // График долготы
    cursor.insertHtml("<h2 style='color: #2c3e50; border-bottom: 2px solid #3498db; padding-bottom: 10px; margin-top: 30px;'><br>"
                      "Динамика долготы</h2>");
    yAxisList->item(4)->setSelected(true);
    generateChartBlock(cursor);
    yAxisList->item(4)->setSelected(false);

    cursor.insertHtml(
        "<br><div style='margin: 20px 0; padding: 15px; background: #f8f9fa; border-radius: 6px;'>"
        "<p style='line-height: 1.6; color: #666;'>"
        "График отображает изменение долготы в течение полета. "
        "Положительные значения соответствуют восточной долготе, отрицательные - западной."
        "</p></div>"
        );

    // Круговая диаграмма валидности
    cursor.insertHtml("<h2 style='color: #2c3e50; border-bottom: 2px solid #3498db; padding-bottom: 10px; margin-top: 30px;'><br>"
                      "Статус данных</h2>");

    generatePieChart(cursor);
    cursor.insertHtml(
        "<br><div style='margin: 20px 0; padding: 15px; background: #f8f9fa; border-radius: 6px;'>"
        "<p style='line-height: 1.6; color: #666;'>"
        "Диаграмма показывает соотношение валидных и невалидных записей данных. "
        "Валидные данные соответствуют корректным показаниям навигационной системы."
        "</p></div>"
        );

    // 3D визуализация
    cursor.insertHtml("<h2 style='color: #2c3e50; border-bottom: 2px solid #3498db; padding-bottom: 10px; margin-top: 30px;'><br>"
                     "Трехмерная модель</h2>");
    generate3DChartBlock(cursor);

    // Описание 3D модели
    cursor.insertHtml(
        "<div style='margin: 20px 0; padding: 15px; background: #f8f9fa; border-radius: 6px;'><br>"
        "<h3 style='color: #34495e; margin-top: 0;'>Особенности 3D визуализации</h3>"
        "<p style='line-height: 1.6; color: #666;'>"
        "Трехмерное представление позволяет анализировать пространственное распределение параметров. "
        "Ось X соответствует долготе, ось Y - высоте, ось Z - широте. "
        "Размер точек пропорционален скорости движения."
        "</p></div>"
    );

    // Таблица данных
    cursor.insertHtml("<h2 style='color: #2c3e50; border-bottom: 2px solid #3498db; padding-bottom: 10px; margin-top: 30px;'><br>"
                     "Исходные данные</h2>");
    generateTableBlock(cursor);

    // Заключение
    cursor.insertHtml(QString(
        "<div style='display: grid; grid-template-columns: 1fr 1fr; gap: 20px;'><br>"
        "<div style='margin-top: 25px; padding-top: 20px; border-top: 1px solid #eee; display: flex; justify-content: space-between;'>"
        "<div style='width: 45%;'>"
        "<div style='font-weight: bold; margin-bottom: 5px;'>Ответственный исполнитель</div>"
        "<div style='border-bottom: 1px solid #ccc; padding-bottom: 5px; margin-bottom: 10px;'></div>"
        "<div style='color: #7f8c8d; font-size: 0.9em;'>Дата: %1</div>"
        "</div>"
        "<div style='width: 45%;'>"
        "<div style='font-weight: bold; margin-bottom: 5px;'>Утверждаю</div>"
        "<div style='border-bottom: 1px solid #ccc; padding-bottom: 5px; margin-bottom: 10px;'></div>"
        "<div style='color: #7f8c8d; font-size: 0.9em;'>Дата: %1</div>"
        "</div></div></div>"
    ).arg(QDate::currentDate().toString("dd.MM.yyyy")));
    m_logger->log(Logger::Debug, "Основные разделы отчета сгенерированы");
    } catch(const std::exception& e) {
        if(m_logger) {
            m_logger->log(Logger::Error,
                          QString("Ошибка при генерации раздела отчета: %1").arg(e.what()));
        }
        throw;
    }
}

// Дополнительные методы
QString ReportTab::formatDuration(int seconds) {
    return QString("%1:%2:%3")
        .arg(seconds / 3600, 2, 10, QLatin1Char('0'))
        .arg((seconds % 3600) / 60, 2, 10, QLatin1Char('0'))
        .arg(seconds % 60, 2, 10, QLatin1Char('0'));
}

std::tuple<double, double, double, double, double, int, double, double, double, double>
ReportTab::calculateFlightMetrics(const QList<NavigationData>& data) {
    double minAlt = std::numeric_limits<double>::max();
    double maxAlt = std::numeric_limits<double>::lowest();
    double minLat = std::numeric_limits<double>::max();
    double maxLat = std::numeric_limits<double>::lowest();
    double minLon = std::numeric_limits<double>::max();
    double maxLon = std::numeric_limits<double>::lowest();
    double speedSum = 0;
    double maxSpeed = 0;
    double courseSum = 0;
    QDateTime startTime, endTime;

    for(const auto& item : data) {
        auto d = item.deserialize();
        QDateTime dt(d.gnzda.date, d.gnzda.time);

        if(dt.isValid()) {
            if(!startTime.isValid() || dt < startTime) startTime = dt;
            if(!endTime.isValid() || dt > endTime) endTime = dt;
        }

        if(d.gngga.altitude > 0) {
            minAlt = qMin(minAlt, static_cast<double>(d.gngga.altitude));
            maxAlt = qMax(maxAlt, static_cast<double>(d.gngga.altitude));
        }

        if(d.gnrmc.speed > 0) {
            speedSum += d.gnrmc.speed;
            maxSpeed = qMax(maxSpeed, d.gnrmc.speed);
        }

        if(d.gnrmc.latitude != 0) {
            minLat = qMin(minLat, d.gnrmc.latitude);
            maxLat = qMax(maxLat, d.gnrmc.latitude);
        }

        if(d.gnrmc.longitude != 0) {
            minLon = qMin(minLon, d.gnrmc.longitude);
            maxLon = qMax(maxLon, d.gnrmc.longitude);
        }

        if(d.gnrmc.course >= 0) {
            courseSum += d.gnrmc.course;
        }
    }

    int duration = startTime.secsTo(endTime);
    return {
        minAlt,
        maxAlt,
        data.isEmpty() ? 0 : speedSum / data.size(),
        maxSpeed,
        data .isEmpty() ? 0 : courseSum / data.size(),
        duration,
        minLat,
        maxLat,
        minLon,
        maxLon
    };
}

// Вспомогательные методы
QString ReportTab::generateLegendHtml() const {
    QStringList items;
    const QList<QColor> colors = {
        QColor(0, 97, 158), QColor(227, 114, 34),
        QColor(89, 161, 79), QColor(186, 60, 61),
        QColor(128, 100, 162)
    };

    int index = 0;
    foreach(QListWidgetItem *item, yAxisList->selectedItems()) {
        items << QString("<li><span style='color: %1;'>■</span> %2</li>")
                     .arg(colors[index++ % colors.size()].name())
                     .arg(item->text());
    }
    return items.join("");
}

double ReportTab::calculateDistance(const QList<NavigationData>& data) {
    if(data.size() < 2) return 0.0;

    auto first = data.first().deserialize();
    auto last = data.last().deserialize();

    // Простая формула расчета расстояния
    const double R = 6371.0; // Радиус Земли в км
    double lat1 = qDegreesToRadians(first.gnrmc.latitude);
    double lon1 = qDegreesToRadians(first.gnrmc.longitude);
    double lat2 = qDegreesToRadians(last.gnrmc.latitude);
    double lon2 = qDegreesToRadians(last.gnrmc.longitude);

    double dlat = lat2 - lat1;
    double dlon = lon2 - lon1;

    double a = qSin(dlat/2) * qSin(dlat/2) +
               qCos(lat1) * qCos(lat2) *
                   qSin(dlon/2) * qSin(dlon/2);
    double c = 2 * qAtan2(qSqrt(a), qSqrt(1-a));

    return R * c;
}

void ReportTab::generateReportHeader() {
    QTextCursor cursor = reportTextEdit->textCursor();

    // Добавляем логотип и заголовок в таблице
    QString header = QString(
                         "<table width='100%' style='border-bottom: 2px solid #3498db; margin-bottom: 20px;'>"
                         "<tr>"
                         "<td width='120' style='padding-right: 20px;'>"
                         "<img src='data:image/png;base64,%1' width='100' />" // Вставка Base64 логотипа
                         "</td>"
                         "<td style='vertical-align: middle;'>"
                         "<h1 style='color: #2c3e50; margin: 0;'>Отчет по полету: %2</h1>"
                         "<p style='color: #7f8c8d; margin: 5px 0 0 0;'>УПКБ &laquo;Деталь&raquo;</p>"
                         "<p style='color: #7f8c8d; margin: 5px 0 0 0;'>Сделано в программном модуле &laquo;Комета&raquo;</p>"
                         "</td>"
                         "</tr>"
                         "</table>"
                         "<br>"
                         ).arg(getLogoBase64()).arg(flightComboBox->currentText());

    cursor.insertHtml(header);

    // Остальной код остается без изменений
    QString filterInfo;
    if(!filterLineEdit->text().isEmpty()) {
        filterInfo = QString("Фильтр: %1 = %2<br>")
                         .arg(filterComboBox->currentText())
                         .arg(filterLineEdit->text());
    }

    QString sortInfo = QString("Сортировка: %1 (%2)<br><br>")
                           .arg(sortComboBox->currentText())
                           .arg(orderComboBox->currentText().toLower());

    cursor.insertHtml(QString("<div style='margin: 15px 0; padding: 10px; background-color: #f8f9fa; border-radius: 5px;'>"
                              "<h3 style='color: #34495e; margin-top: 0;'>Параметры отчета:</h3>"
                              "%1%2</div>")
                          .arg(filterInfo)
                          .arg(sortInfo));
}

QString ReportTab::getLogoBase64() {
    QImage logo(":/data/Resources/upkbLogo.png"); // Путь к изображению в ресурсах
    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    logo.save(&buffer, "PNG");
    return QString(byteArray.toBase64());
}

void ReportTab::generateTableBlock(QTextCursor &cursor) {
    QList<NavigationData> data = getFilteredData();
    if(data.isEmpty()){
        m_logger->log(Logger::Warning, "Пустые данные для генерации таблицы");
        return;
    }
    m_logger->log(Logger::Debug,
                  QString("Генерация таблицы для %1 записей").arg(data.size()));
    try {
        // Получаем выбранные колонки
        QList<QCheckBox*> columnCheckBoxes = tableSettingsGroup->findChildren<QCheckBox*>();
        QStringList selectedColumns;
        for(QCheckBox *cb : columnCheckBoxes) {
            if(cb->isChecked()) {
                selectedColumns << cb->text();
            }
        }
        if(selectedColumns.isEmpty()) return;

        // Создаем HTML таблицу
        // Стили для таблицы
        const QString tableStyle =
            "border='1'"
            "style="
            "'font-size: 9px;"          // Размер шрифта
            "border-collapse: collapse;"
            "width: 100%;"
            "margin: 8px 0;"
            "font-family: Arial;'";     // Шрифт

        // Создаем HTML таблицу
        QString html = "<table " + tableStyle + ">";

        // Заголовки
        html += "<tr style='background-color: #f2f2f2;'>";
        foreach(const QString &col, selectedColumns) {
            html += QString("<th style='padding: 8px;'>%1</ th>").arg(col);
        }
        html += "</tr>";

        // Данные
        foreach(const auto &item, data) {
            auto deserialized = item.deserialize();
            QDateTime dt(deserialized.gnzda.date, deserialized.gnzda.time);

            html += "<tr>";
            foreach(const QString &col, selectedColumns) {
                QString value;
                if(col == "ID") value = QString::number(deserialized.id);
                else if(col == "Время (UTC)") value = dt.toString("HH:mm:ss");
                else if(col == "Дата") value = dt.toString("dd.MM.yyyy");
                else if(col == "Широта (°)") value = QString::number(deserialized.gnrmc.latitude, 'f', 6);
                else if(col == "Долгота (°)") value = QString::number(deserialized.gnrmc.longitude, 'f', 6);
                else if(col == "Высота (м)") value = QString::number(deserialized.gngga.altitude, 'f', 1);
                else if(col == "Скорость (м/с)") value = QString::number(deserialized.gnrmc.speed, 'f', 1);
                else if(col == "Курс (°)") value = QString::number(deserialized.gnrmc.course, 'f', 1);
                else if(col == "Действителен") value = deserialized.gnrmc.isValid ? "Да" : "Нет";

                html += QString("<td style='padding: 6px; border: 1px solid #ddd;'>%1</td>").arg(value);
            }
            html += "</tr>";
        }
        html += "</table><br>";

        cursor.insertHtml(html);

        QString caption = QString("<div style='margin: 15px 0; color: #34495e;'>"
                                  "<h3>Таблица данных</h3>"
                                  "<p>Количество записей: %1</p></div>")
                              .arg(data.count());
        cursor.insertHtml(caption);
        m_logger->log(Logger::Info, "Блок таблицы успешно сгенерирован");
    } catch(const std::exception& e) {
        if(m_logger) {
            m_logger->log(Logger::Error,
                          QString("Ошибка при генерации таблицы: %1").arg(e.what()));
        }
        throw;
    }
}

void ReportTab::generateChartBlock(QTextCursor &cursor) {
    QList<NavigationData> data = getFilteredData();
    if(data.isEmpty()){
        m_logger->log(Logger::Warning, "Пустые данные для генерации графика");
        return;
    }

    m_logger->log(Logger::Debug,
                  QString("Генерация графика для %1 записей").arg(data.size()));
    try {
    // Фильтрация и подготовка данных
    QVector<QDateTime> timestamps;
    QMap<QString, QVector<double>> values;

    for (const NavigationData &navData : data) {
        auto d = navData.deserialize();
        QDateTime dt(d.gnzda.date, d.gnzda.time);
        if (!dt.isValid()) continue;

        timestamps.append(dt);
        values["Широта"].append(d.gnrmc.latitude);
        values["Долгота"].append(d.gnrmc.longitude);
        values["Высота"].append(d.gngga.altitude);
        values["Скорость"].append(d.gnrmc.speed);
        values["Курс"].append(d.gnrmc.course);
    }

    if (timestamps.isEmpty()) {
        qDebug() << "Нет валидных временных меток";
        return;
    }

    // Создание графика
    QtCharts::QChart *chart = new QtCharts::QChart();
    chart->setBackgroundBrush(Qt::white);
    chart->setMargins(QMargins(20, 10, 20, 15));
    chart->setContentsMargins(0, 0, 0, 0);

    // Ось времени
    QtCharts::QDateTimeAxis *axisX = new QtCharts::QDateTimeAxis();
    axisX->setFormat("dd.MM.yy HH:mm");
    axisX->setTitleText("Временная шкала");
    axisX->setTitleFont(QFont("Arial", 10));
    axisX->setLabelsFont(QFont("Arial", 8));
    axisX->setLabelsAngle(-30);
    axisX->setGridLineVisible(true);
    chart->addAxis(axisX, Qt::AlignBottom);

    // Числовая ось
    QtCharts::QValueAxis *axisY = new QtCharts::QValueAxis();
    axisY->setTitleText("Значения параметров");
    axisY->setTitleFont(QFont("Arial", 10));
    axisY->setLabelsFont(QFont("Arial", 8));
    axisY->setLabelFormat("%.2f");
    axisY->setGridLineVisible(true);
    chart->addAxis(axisY, Qt::AlignLeft);

    // Цвета для линий
    const QList<QColor> lineColors = {
        QColor(0, 97, 158),    // Синий
        QColor(227, 114, 34),  // Оранжевый
        QColor(89, 161, 79),   // Зеленый
        QColor(186, 60, 61),   // Красный
        QColor(128, 100, 162)  // Фиолетовый
    };

    // Создание серий в зависимости от выбранного типа
    int colorIndex = 0;
    QList<QtCharts::QXYSeries*> allSeries;
    QString chartType = chartTypeSelector->currentText();

    if(chartType == "Круговой") {
        generatePieChart(cursor);
        return;
    }

    foreach (QListWidgetItem *item, yAxisList->selectedItems()) {
        const QString param = item->text();
        if (!values.contains(param)) continue;

        QtCharts::QXYSeries *series = nullptr;

        // Создаем серию нужного типа
        if(chartType == "Линейный") {
            series = new QtCharts::QLineSeries();
        }
        else if(chartType == "Spline") {
            series = new QtCharts::QSplineSeries();
        }
        else if(chartType == "Точечный") {
            auto scatter = new QtCharts::QScatterSeries();
            scatter->setMarkerSize(8);
            series = scatter;
        }

        series->setName(param);

        // Настройка стиля
        QPen pen(lineColors[colorIndex % lineColors.size()]);
        pen.setWidth(2);
        pen.setCosmetic(true);
        series->setPen(pen);

        if(auto scatter = qobject_cast<QtCharts::QScatterSeries*>(series)) {
            scatter->setBorderColor(pen.color());
            scatter->setBrush(pen.color());
        }

        colorIndex++;

        // Заполнение данных
        for (int i = 0; i < timestamps.size(); ++i) {
            const double value = values[param][i];
            if (!qFuzzyIsNull(value)) {
                series->append(timestamps[i].toMSecsSinceEpoch(), value);
            }
        }

        if (series->count() > 0) {
            chart->addSeries(series);
            allSeries.append(series);
        } else {
            delete series;
        }
    }

    if (allSeries.isEmpty()) {
        delete chart;
        return;
    }

    // Привязка осей ко всем сериям
    foreach (QtCharts::QXYSeries *series, allSeries) {
        series->attachAxis(axisX);
        series->attachAxis(axisY);
    }

    // Автомасштабирование для оси Y
    double yMin = std::numeric_limits<double>::max();
    double yMax = std::numeric_limits<double>::lowest();

    foreach (const QtCharts::QXYSeries *series, allSeries) {
        const auto points = series->points();
        for (const QPointF &p : points) {
            yMin = qMin(yMin, p.y());
            yMax = qMax(yMax, p.y());
        }
    }

    if (yMax > yMin) {
        const double yPadding = (yMax - yMin) * 0.05;
        axisY->setRange(yMin - yPadding, yMax + yPadding);
    } else {
        axisY->setRange(yMin - 1.0, yMax + 1.0);
    }

    // Настройка легенды
    QtCharts::QLegend *legend = chart->legend();
    legend->setVisible(true);
    legend->setAlignment(Qt::AlignBottom);
    legend->setBackgroundVisible(true);
    legend->setBrush(QColor(255, 255, 255, 200));
    legend->setFont(QFont("Arial", 9));

    // Рендеринг
    QtCharts::QChartView chartView;
    chartView.setChart(chart);
    chartView.setRenderHint(QPainter::Antialiasing);
    chartView.resize(800, 500);

    QPixmap pixmap = chartView.grab();
    insertPixmapToDocument(pixmap, cursor);

    // Добавляем подпись к графику
    QString chartTitle = chart->title();
    QString caption = QString(
                          "<br><div style='text-align: center; margin: 10px 0; font-style: italic; color: #7f8c8d;'>"
                          "Рисунок %1: График - %2</div>"
                          ).arg(++figureCounter).arg(chartTitle.isEmpty() ? "Динамика параметров" : chartTitle);

    cursor.insertHtml(caption);

    // Очистка памяти
    delete chart;
    m_logger->log(Logger::Debug, "Блок графиков успешно сгенерированы");
    } catch(const std::exception& e) {
        if(m_logger) {
            m_logger->log(Logger::Error,
                          QString("Ошибка генерации блока графика: %1").arg(e.what()));
        }
        throw;
    }
}

void ReportTab::generatePieChart(QTextCursor &cursor) {
    QList<NavigationData> data = getFilteredData();
    if(data.isEmpty()) return;

    QtCharts::QPieSeries *series = new QtCharts::QPieSeries();

    // Пример для круговой диаграммы: распределение по статусам валидности
    int validCount = 0, invalidCount = 0;
    foreach(const auto &item, data) {
        if(item.deserialize().gnrmc.isValid) validCount++;
        else invalidCount++;
    }

    if(validCount > 0) {
        QtCharts::QPieSlice *validSlice = series->append("Валидные", validCount);
        validSlice->setColor(QColor(76, 175, 80));
    }
    if(invalidCount > 0) {
        QtCharts::QPieSlice *invalidSlice = series->append("Невалидные", invalidCount);
        invalidSlice->setColor(QColor(244, 67, 54));
    }

    QtCharts::QChart *chart = new QtCharts::QChart();
    chart->addSeries(series);
    chart->setTitle("Распределение по валидности данных");
    chart->legend()->setVisible(true);

    QtCharts::QChartView chartView(chart);
    chartView.setRenderHint(QPainter::Antialiasing);
    chartView.resize(600, 400);

    QPixmap pixmap = chartView.grab();
    insertPixmapToDocument(pixmap, cursor);

    // Добавляем подпись к графику
    QString chartTitle = chart->title();
    QString caption = QString(
                          "<br><div style='text-align: center; margin: 10px 0; font-style: italic; color: #7f8c8d;'>"
                          "Рисунок %1: График - %2</div>"
                          ).arg(++figureCounter).arg(chartTitle.isEmpty() ? "Динамика параметров" : chartTitle);
    cursor.insertHtml(caption);

    delete chart;
}

QPixmap ReportTab::renderChartToPixmap(QtCharts::QChart *chart) {
    QtCharts::QChartView chartView(chart);
    chartView.setRenderHint(QPainter::Antialiasing);
    chartView.resize(800, 400);
    return chartView.grab();
}

void ReportTab::generateTextBlock(QTextCursor &cursor) {
    m_logger->log(Logger::Debug,
                  QString("Генерация текста..."));
    try {
    QString text = textEdit->toPlainText();
    if(!text.isEmpty()) {
        // Автоматическое форматирование переносов
        text = text.replace("\n", "<br>");

        cursor.insertHtml(QString("<div style='margin: 15px 0; padding: 15px; "
                                  "background-color: #f8f9fa; border-left: 4px solid #3498db; "
                                  "border-radius: 4px;'>"
                                  "<h3 style='color: #2c3e50; margin-top: 0;'>Текстовый блок</h3>"
                                  "<p style='line-height: 1.6;'>%1</p></div>")
                              .arg(text));
    }
    m_logger->log(Logger::Info, "Блок текста успешно сгенерирован");
    } catch(const std::exception& e) {
        if(m_logger) {
            m_logger->log(Logger::Error,
                          QString("Ошибка генерации блока текста: %1").arg(e.what()));
        }
    }
}

void ReportTab::selectChartColor() {
    QColor newColor = QColorDialog::getColor(chartColor, this, "Выберите цвет графика");
    if(newColor.isValid()) {
        chartColor = newColor;
        colorButton->setStyleSheet(QString("background-color: %1;").arg(chartColor.name()));
    }
}

QList<NavigationData> ReportTab::getFilteredData() {
    QString filterField = filterComboBox->currentText();
    QString filterValue = filterLineEdit->text();
    QString sortField = sortComboBox->currentText();
    QString sortOrder = orderComboBox->currentText() == "По возрастанию" ? "ASC" : "DESC";

    return dbManager->getNavigationDataFilter(
        filterField,
        filterValue,
        sortField,
        sortOrder,
        flightComboBox->currentText()
        );
}

void ReportTab::setup3DChart(QtDataVisualization::Q3DScatter *chart, const QList<NavigationData> &data) {    
    // Настройка фона и темы
    chart->activeTheme()->setType(QtDataVisualization::Q3DTheme::ThemeQt);
    chart->activeTheme()->setBackgroundEnabled(false);
    chart->activeTheme()->setGridLineColor(Qt::gray);

    // Настройка освещения
    chart->activeTheme()->setLightStrength(5.0f);
    chart->activeTheme()->setAmbientLightStrength(0.5f);
    chart->activeTheme()->setLightColor(Qt::white);

    // Создание и настройка серии данных
    QtDataVisualization::QScatter3DSeries *series = new QtDataVisualization::QScatter3DSeries();
    series->setItemSize(scatterPointSize / 100.0f);
    series->setBaseColor(scatterColor);

    // Заполнение данных
    QtDataVisualization::QScatterDataArray *dataArray = new QtDataVisualization::QScatterDataArray();
    int counter = 0;
    foreach (const NavigationData &item, data) {
        if (counter++ % scatterDensity != 0) continue;
        auto deserialized = item.deserialize();
        if (deserialized.gnrmc.isValid) {
            dataArray->append(QtDataVisualization::QScatterDataItem(
                QVector3D(deserialized.gnrmc.longitude,
                          deserialized.gngga.altitude,
                          deserialized.gnrmc.latitude)));
        }
    }

    series->dataProxy()->resetArray(dataArray);
    chart->addSeries(series);

    // Настройка камеры
    chart->scene()->activeCamera()->setCameraPreset(QtDataVisualization::Q3DCamera::CameraPresetIsometricLeft);
    chart->setOrthoProjection(cameraProjectionCombo->currentIndex() == 1);
}

QPixmap ReportTab::render3DChartToPixmap(QtDataVisualization::Q3DScatter *chart) {
    // Рендерим в изображение с высоким разрешением
    QImage image = chart->renderToImage(8, QSize(1200, 800));

    // Конвертируем в Pixmap и возвращаем
    return QPixmap::fromImage(image);
}

void ReportTab::generate3DChartBlock(QTextCursor &cursor) {
    QList<NavigationData> data = getFilteredData();
    if(data.isEmpty()){
        m_logger->log(Logger::Warning, "Пустые данные для генерации графика");
        return;
    }

    m_logger->log(Logger::Debug,
                  QString("Начало генерации блока 3d графика"));
    try {

    QtDataVisualization::Q3DScatter *chart = new QtDataVisualization::Q3DScatter();
    setup3DChart(chart, data);

    // Даем время на завершение рендеринга
    QApplication::processEvents();

    QPixmap pixmap = render3DChartToPixmap(chart);
    insertPixmapToDocument(pixmap, cursor);

    delete chart; // Очищаем ресурсы

    QString caption = QString("<br><div style='text-align: center; margin: 10px 0; font-style: italic; color: #7f8c8d;'>"
                              "Рисунок %1: 3D визуализация маршрута</div>")
                          .arg(++figureCounter);
    cursor.insertHtml(caption);
    m_logger->log(Logger::Info,
                  QString("Блок 3d графика успешно сгенерирован"));
} catch(const std::exception& e) {
    if(m_logger) {
        m_logger->log(Logger::Error,
                      QString("Ошибка генерации блока 3d графика: %1").arg(e.what()));
    }
    QMessageBox::critical(this, "Ошибка", "Не удалось сгенерировать блок 3d график");
}
}

void ReportTab::onAIAnalysisComplete(const QString &result) {
    QTextCursor cursor(reportTextEdit->textCursor());
    cursor.movePosition(QTextCursor::End);

    cursor.insertHtml(QString("<div style='margin: 15px 0; padding: 15px; "
                              "background-color: #f8f9fa; border-left: 4px solid #27ae60; "
                              "border-radius: 4px;'>"
                              "<h3 style='color: #2c3e50; margin-top: 0;'>Анализ ИИ</h3>"
                              "<p style='line-height: 1.6;'>%1</p></div>")
                          .arg(result));
}

void ReportTab::onAIError(const QString &message) {
    qWarning() << "AI Error:" << message;
    QMessageBox::warning(this, "Ошибка AI", message);
}

void ReportTab::insertPixmapToDocument(const QPixmap &pixmap, QTextCursor &cursor) {
    // Оптимальные размеры для A4 (ширина 800px при 300 DPI)
    const int maxWidth = 400;
    const int maxHeight = 400;

    // Масштабируем изображение с сохранением пропорций
    QPixmap scaledPix = pixmap.scaled(maxWidth, maxHeight,
                                      Qt::KeepAspectRatio,
                                      Qt::SmoothTransformation);

    // Конвертируем в Base64
    QByteArray imageData;
    QBuffer buffer(&imageData);
    buffer.open(QIODevice::WriteOnly);
    scaledPix.save(&buffer, "PNG");

    // Вставка с центрированием
    cursor.insertHtml(QString(
                          "<br><div style='text-align: center; margin: 15px 0;'>"
                          "<img src='data:image/png;base64,%0' width='%1'/>"
                          "</div>"
                          ).arg(QString(imageData.toBase64()))
                          .arg(scaledPix.width()));
    m_logger->log(Logger::Info,
                  QString("картинка успешно вставлена"));
}

void ReportTab::onSaveReport() {
    m_logger->log(Logger::Info, "Попытка сохранения отчета");
    QStringList formats = {"HTML", "PDF"};
    bool ok;
    QString format = QInputDialog::getItem(this, "Выберите формат", "Формат сохранения:", formats, 0, false, &ok);

    if (ok && !format.isEmpty()) {
        QString fileName = QFileDialog::getSaveFileName(this, "Сохранить отчет как", "",
                                                        QString("%1 Files (*.%1)").arg(format.toLower()));

        if (fileName.isEmpty()){
            m_logger->log(Logger::Debug, "Сохранение отчета отменено пользователем");
            return;
        }
        try{
        // Сохраняем отчет в файл в зависимости от выбранного формата
        if (format == "HTML") {
            saveAsHtml(fileName);
        } else if (format == "PDF") {
            saveAsPdf(fileName);
        } else if (format == "DOCX") {
            saveAsDocx(fileName);
        } else {
            m_logger->log(Logger::Info,
                          QString("Отчет успешно сохранен в файл: %1").arg(fileName));
            QMessageBox::warning(this, "Ошибка", "Неподдерживаемый формат файла");
        }
        } catch(const std::exception& e) {
            if(m_logger) {
                m_logger->log(Logger::Error,
                              QString("Ошибка сохранения отчета: %1").arg(e.what()));
            }
            QMessageBox::critical(this, "Ошибка", "Не удалось сохранить отчет");
        }
    }
}

void ReportTab::saveAsHtml(const QString &fileName) {
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly)) {
        QTextStream stream(&file);
        stream.setCodec("UTF-8");
        stream << reportTextEdit->toHtml();
        file.close();
    }
}

void ReportTab::saveAsPdf(const QString &fileName) {
    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fileName);
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setPageMargins(QMarginsF(15, 15, 15, 15), QPageLayout::Millimeter);

    // Рендерим с учетом изображений
    QTextDocument doc;
    doc.setHtml(reportTextEdit->toHtml());
    doc.setPageSize(printer.pageRect(QPrinter::Point).size());
    doc.print(&printer);
}

void ReportTab::saveAsDocx(const QString &fileName) {
    // 1. Сохраняем отчет во временный HTML-файл
    QTemporaryFile tempFile;
    if (!tempFile.open()) {
        QMessageBox::critical(this, "Ошибка", "Не удалось создать временный файл");
        return;
    }

    QString htmlContent = reportTextEdit->toHtml();
    tempFile.write(htmlContent.toUtf8());
    tempFile.close();

    // 2. Конвертируем HTML в DOCX используя LibreOffice
    QStringList args;
    args << "--headless"
         << "--convert-to" << "docx"
         << "--outdir" << QFileInfo(fileName).path()
         << tempFile.fileName();

    QProcess process;
    process.start("libreoffice", args);

    if (!process.waitForFinished(30000)) { // Таймаут 30 секунд
        QMessageBox::critical(this, "Ошибка",
                              "Не удалось выполнить конвертацию. Убедитесь, что LibreOffice установлен.");
        return;
    }

    // 3. Переименовываем результат
    QString tempOutput = QFileInfo(tempFile.fileName()).path() + "/" +
                         QFileInfo(tempFile.fileName()).baseName() + ".docx";

    if (QFile::exists(tempOutput)) {
        QFile::remove(fileName);
        QFile::rename(tempOutput, fileName);
    }
}

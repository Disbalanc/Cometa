#include "setupgraphtab.h"
#include <QFileDialog>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QComboBox>
#include <QSpinBox>
#include <QLabel>
#include <QColorDialog>
#include <QDebug>
#include <QFormLayout>
#include <Q3DCamera>
#include <QPrinter>
#include <QPainter>
#include <QtDataVisualization/Q3DScatter>
#include <QtDataVisualization/QScatter3DSeries>
#include <QtDataVisualization/QScatterDataProxy>
#include <QtDataVisualization/QScatterDataArray>

setupGraphTab::setupGraphTab(DatabaseManager *db, Logger *logger,QWidget *parent) : QWidget(parent),dbManager(db),m_logger(logger) {
    m_logger->log(Logger::Info, "Инициализация 3D графика...");
    setupUI();
}

void setupGraphTab::setupUI() {
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(5, 5, 5, 5);

    // Создание 3D графика
    scatterGraph = new QtDataVisualization::Q3DScatter();
    auto *container = QWidget::createWindowContainer(scatterGraph);
    mainLayout->addWidget(container, 1);

    // Настройка 3D графика
    scatterGraph->setShadowQuality(QtDataVisualization::QAbstract3DGraph::ShadowQualityMedium);
    scatterGraph->setAxisX(new QtDataVisualization::QValue3DAxis);
    scatterGraph->setAxisZ(new QtDataVisualization::QValue3DAxis);
    scatterGraph->setAxisY(new QtDataVisualization::QValue3DAxis);

    // Создание серии данных
    series = new QtDataVisualization::QScatter3DSeries();
    scatterGraph->addSeries(series);

    // Панель параметров
    auto *parameterPanel = new QWidget(this);
    auto *parameterLayout = new QVBoxLayout(parameterPanel);
    parameterLayout->setContentsMargins(0, 0, 0, 0);

    // Верхняя строка: Фильтрация и сортировка
    auto *topRow = new QHBoxLayout();

    // Фильтрация
    auto *filterGroup = new QGroupBox("Фильтр", this);
    auto *filterLayout = new QVBoxLayout(filterGroup);
    filterComboBox = new QComboBox(this);
    filterComboBox->addItems({"ID", "Time", "Date", "Latitude", "Longitude", "Altitude", "Speed", "Course", "IsValid", "TimeStamp"});
    filterLayout->addWidget(filterComboBox);
    filterLineEdit = new QLineEdit(this);
    filterLayout->addWidget(filterLineEdit);
    topRow->addWidget(filterGroup);

    // Сортировка
    auto *sortGroup = new QGroupBox("Сортировка", this);
    auto *sortLayout = new QHBoxLayout(sortGroup);
    sortComboBox = new QComboBox(this);
    sortComboBox->addItems({"ID", "Time", "Date", "Latitude", "Longitude", "Altitude", "Speed", "Course", "IsValid", "TimeStamp"});
    sortLayout->addWidget(sortComboBox);
    orderComboBox = new QComboBox(this);
    orderComboBox->addItems({"По возрастанию", "По убыванию"});
    sortLayout->addWidget(orderComboBox);
    topRow->addWidget(sortGroup);

    parameterLayout->addLayout(topRow);

    // Средняя строка: Выбор полета и кнопки
    auto *middleRow = new QHBoxLayout();

    // Выбор полета
    flightComboBox = new QComboBox(this);
    middleRow->addWidget(createLabel("Полет:"));
    middleRow->addWidget(flightComboBox);

    // Кнопки
    auto *buttonGroup = new QGroupBox("Действия", this);
    auto *buttonLayout = new QHBoxLayout(buttonGroup);
    auto *applyButton = new QPushButton("Загрузить", this);
    buttonLayout->addWidget(applyButton);
    auto *resetButton = new QPushButton("Сброс", this);
    buttonLayout->addWidget(resetButton);
    middleRow->addWidget(buttonGroup);

    parameterLayout->addLayout(middleRow);

    // Нижняя строка: Настройки графика
    auto *bottomRow = new QHBoxLayout();

    // Настройки проекции
    auto *projectionGroup = new QGroupBox("Проекция", this);
    auto *projectionLayout = new QFormLayout(projectionGroup);

    // Выбор типа проекции
    QComboBox *projectionTypeCombo = new QComboBox(this);
    projectionTypeCombo->addItems({"Перспектива", "Изометрия"});
    projectionLayout->addRow("Тип проекции:", projectionTypeCombo);

    // Выбор положения камеры
    QComboBox *cameraPresetCombo = new QComboBox(this);
    cameraPresetCombo->addItems({"Фронтальная", "Вид сверху", "Изометрия"});
    projectionLayout->addRow("Положение камеры:", cameraPresetCombo);

    bottomRow->addWidget(projectionGroup);

    // Настройки отображения
    auto *displayGroup = new QGroupBox("Отображение", this);
    auto *displayLayout = new QVBoxLayout(displayGroup);

    // Размер точек
    auto *sizeLayout = new QHBoxLayout();
    auto *sizeSpinBox = new QSpinBox(this);
    sizeSpinBox->setRange(1, 100);
    sizeSpinBox->setValue(50);
    sizeLayout->addWidget(new QLabel("Размер:"));
    sizeLayout->addWidget(sizeSpinBox);
    displayLayout->addLayout(sizeLayout);

    // Цвет точек
    auto *colorButton = new QPushButton("Выбрать цвет", this);
    displayLayout->addWidget(colorButton);

    // Плотность точек
    auto *densityLayout = new QHBoxLayout();
    densitySpinBox = new QSpinBox(this);
    densitySpinBox->setRange(1, 100);
    densitySpinBox->setValue(5);
    densityLayout->addWidget(new QLabel("Плотность:"));
    densityLayout->addWidget(densitySpinBox);
    displayLayout->addLayout(densityLayout);

    bottomRow->addWidget(displayGroup);

    // В секции с кнопками (внутри setupUI)
    auto *saveGroup = new QGroupBox("Сохранение", this);
    auto *saveLayout = new QHBoxLayout(saveGroup);

    // Выбор формата
    QComboBox *formatCombo = new QComboBox(this);
    formatCombo->addItems({"PNG", "JPEG", "BMP", "PDF"});

    // Кнопка сохранения
    QPushButton *saveButton = new QPushButton("Сохранить", this);

    saveLayout->addWidget(new QLabel("Формат:"));
    saveLayout->addWidget(formatCombo);
    saveLayout->addWidget(saveButton);

    // Добавляем группу сохранения в layout
    bottomRow->addWidget(saveGroup);

    parameterLayout->addLayout(bottomRow);

    mainLayout->addWidget(parameterPanel);

    // Подключение сигналов
    connect(projectionTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int index) {
                bool isOrtho = (index == 1);
                scatterGraph->setOrthoProjection(isOrtho);
            });

    connect(cameraPresetCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int index) {
                using namespace QtDataVisualization;
                Q3DCamera::CameraPreset preset = Q3DCamera::CameraPresetFront;
                switch(index) {
                case 0: preset = Q3DCamera::CameraPresetFront; break;
                //case 1: preset = Q3DCamera::CameraPresetTop; break;
                case 2: preset = Q3DCamera::CameraPresetIsometricLeft; break;
                }
                scatterGraph->scene()->activeCamera()->setCameraPreset(preset);
            });

    connect(sizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, [this](int size) {
                series->setItemSize(size / 100.0f);
            });

    connect(colorButton, &QPushButton::clicked, this, [this]() {
        QColor color = QColorDialog::getColor(Qt::blue, this);
        if (color.isValid()) {
            series->setBaseColor(color);
        }
    });

    connect(saveButton, &QPushButton::clicked, this, [this, formatCombo]() {
        QString format = formatCombo->currentText().toLower();
        QString fileName = QFileDialog::getSaveFileName(
            this,
            "Сохранить график",
            "",
            QString("%1 Files (*.%2)").arg(format.toUpper()).arg(format)
            );

        if (!fileName.isEmpty()) {
            try{
            // Создаем изображение с текущим видом камеры
            QImage image = scatterGraph->renderToImage(8, QSize(1024, 768));

            if (format == "pdf") {
                // Для PDF используем QPrinter
                QPrinter printer(QPrinter::HighResolution);
                printer.setOutputFormat(QPrinter::PdfFormat);
                printer.setOutputFileName(fileName);

                QPainter painter(&printer);
                QRect rect = printer.pageRect();
                painter.drawImage(rect, image);
                painter.end();
            } else {
                // Для растровых форматов сохраняем напрямую
                image.save(fileName, format.toUpper().toLatin1());
            }

            m_logger->log(Logger::Info,
                          QString("График успешно сохранен в файл: %1").arg(fileName));
            }
            catch(const std::exception& e) {
                    m_logger->log(Logger::Error,
                                  QString("Ошибка сохранения графика: %1").arg(e.what()));
            }
        }
    });

    // Подключение сигналов
    connect(densitySpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &setupGraphTab::updatePointDensity);
    connect(applyButton, &QPushButton::clicked, this, &setupGraphTab::applyFilter);
    connect(resetButton, &QPushButton::clicked, this, &setupGraphTab::resetFilters);

    // Заполнение списка полетов
    QList<QString> flights = dbManager->getAllFlights();
    for (const QString &flight : flights) {
        flightComboBox->addItem(flight);
    }
    m_logger->log(Logger::Info, "Интерфейс 3d графиков успешно настроен, количество полетов: " + QString::number(flights.size()));
}

QLabel* setupGraphTab::createLabel(const QString &text) {
    auto *label = new QLabel(text, this);
    label->setStyleSheet("font-weight: bold; color: #333;"); // Установка жирного шрифта для меток
    return label;
}

void setupGraphTab::updatePointDensity(int density) {
    m_logger->log(Logger::Debug, "Изменение плотности точек: " + QString::number(density));
    this->density = density; // Сохраняем значение плотности
    applyFilter(); // Обновляем график
}

void setupGraphTab::resetFilters() {
     m_logger->log(Logger::Info, "Сброс фильтров");
    filterLineEdit->clear(); // Очищаем поле фильтрации
    sortComboBox->setCurrentIndex(0); // Сбрасываем сортировку на первое поле
    filterComboBox->setCurrentIndex(0); // Сбрасываем фильтрацию на первое поле
    orderComboBox->setCurrentIndex(0); // Сбрасываем направление сортировки на "По возрастанию"

    // Вызываем метод для обновления данных без фильтрации
    applyFilter();
}

void setupGraphTab::applyFilter() {
    m_logger->log(Logger::Info,
                  QString("Применение фильтров [Поле: %1, Значение: %2, Сортировка: %3, Порядок: %4]")
                      .arg(filterComboBox->currentText())
                      .arg(filterLineEdit->text())
                      .arg(sortComboBox->currentText())
                      .arg(orderComboBox->currentText()));

    QString filterField = filterComboBox->currentText();
    QString filterValue = filterLineEdit->text();
    QString sortField = sortComboBox->currentText();
    QString sortOrder = orderComboBox->currentText(); // Получаем направление сортировки
    QString flightName = flightComboBox->currentText(); // Получаем текущее имя полета
    density = densitySpinBox->value();

    // Преобразуем направление сортировки в SQL-формат
    sortOrder = (sortOrder == "По возрастанию") ? "ASC" : "DESC"; // Преобразуем в ASC или DESC

    // Получаем отфильтрованные данные
    QList<NavigationData> filteredData = dbManager->getNavigationDataFilterValid(filterField, filterValue, sortField, sortOrder, flightName);
    m_logger->log(Logger::Info,
                  QString("Получено %1 записей для 3d графика").arg(filteredData.size()));
    // Обновляем график с новыми данными
    updateScatterGraph(filteredData);
}

void setupGraphTab::updateScatterGraph(QList<NavigationData> navigationDataList) {
    m_logger->log(Logger::Debug, "Обновление 3D графика...");

    auto *dataProxy = new QtDataVisualization::QScatterDataProxy();
    auto *dataArray = new QtDataVisualization::QScatterDataArray();

    // Инициализация минимальных и максимальных значений
    float minLongitude = std::numeric_limits<float>::max();
    float maxLongitude = std::numeric_limits<float>::min();
    float minLatitude = std::numeric_limits<float>::max();
    float maxLatitude = std::numeric_limits<float>::min();
    float minAltitude = std::numeric_limits<float>::max();
    float maxAltitude = std::numeric_limits<float>::min();

    int validPoints = 0;
    int invalidPoints = 0;
    for (int i = 0; i < navigationDataList.count(); i++) {
        if (i % density == 0) { // Используем плотность для выборки данных
            NavigationData data = navigationDataList[i];
            GNRMCData gnrmc;
            GNGGAData gngga;
            GNZDAData gnzda;
            QDataStream stream(data.data);

            // Попробуем извлечь данные
            try {
                stream >> data.id
                    >> data.timestamp
                    >> gnzda.date
                    >> gnzda.time
                    >> gnrmc.isValid
                    >> gngga.altitude
                    >> gnrmc.latitude
                    >> gnrmc.longitude
                    >> gnrmc.speed
                    >> gnrmc.course;
                // Проверяем валидность данных
                if (gnrmc.isValid) {
                    // Проверяем валидность высоты
                    if (gngga.altitude > 0) {
                        maxLongitude = std::max(maxLongitude, static_cast<float>(gnrmc.longitude));
                        minLongitude = std::min(minLongitude, static_cast<float>(gnrmc.longitude));
                        minLatitude = std::min(minLatitude, static_cast<float>(gnrmc.latitude));
                        maxLatitude = std::max(maxLatitude, static_cast<float>(gnrmc.latitude));
                        minAltitude = std::min(minAltitude, static_cast<float>(gngga.altitude));
                        maxAltitude = std::max(maxAltitude, static_cast<float>(gngga.altitude));

                        // Добавляем точки в массив (долгота, широта, высота)
                        dataArray->append(QtDataVisualization::QScatterDataItem(QVector3D(gnrmc.longitude,
                                                                                          gngga.altitude,
                                                                                          gnrmc.latitude
                                                                                          )));
                                            validPoints++;
                    }else {
                        invalidPoints++;
                    }
                }}catch (const std::exception &e) {
                        m_logger->log(Logger::Warning,
                                      QString("Ошибка обработки записи %1: %2").arg(i).arg(e.what()));
                    invalidPoints++;
                }
        }
    }

    // Проверяем, есть ли данные для отображения
    if (dataArray->size() == 0) {
        m_logger->log(Logger::Warning, "Нет валидных данных для отображения");
        return; // Если данных нет, выходим из функции
    }

    // Сбрасываем текущие данные и добавляем новые
    dataProxy->resetArray(dataArray); // Передаем массив данных

    series->setDataProxy(dataProxy); // Устанавливаем прокси для серии
    series->setBaseColor(color); // Устанавливаем цвет точек
    series->setItemSize(0.2f); // Устанавливаем размер точек
    scatterGraph->addSeries(series); // Добавляем серию точек в график

    // Настройка стиля графика
    scatterGraph->setShadowQuality(QtDataVisualization::QAbstract3DGraph::ShadowQuality::ShadowQualityNone);
    scatterGraph->setAxisX(new QtDataVisualization::QValue3DAxis);
    scatterGraph->setAxisZ(new QtDataVisualization::QValue3DAxis);
    scatterGraph->setAxisY(new QtDataVisualization::QValue3DAxis);

    // Установка диапазонов осей
    if (minLongitude != std::numeric_limits<float>::max() && maxLongitude != std::numeric_limits<float>::min()) {
        scatterGraph->axisX()->setRange(minLongitude, maxLongitude);
    }
    if (minLatitude != std::numeric_limits<float>::max() && maxLatitude != std::numeric_limits<float>::min()) {
        scatterGraph->axisZ()->setRange(minLatitude, maxLatitude);
    }
    if (minAltitude != std::numeric_limits<float>::max() && maxAltitude != std::numeric_limits<float>::min()) {
        scatterGraph->axisY()->setRange(minAltitude, maxAltitude);
    }

    m_logger->log(Logger::Info,
                  QString("График обновлен. Валидных точек: %1, Невалидных: %2")
                      .arg(validPoints).arg(invalidPoints));
}

void setupGraphTab::logMessage(const QString &message) {
    // Логирование сообщений в файл или консоль
    qDebug() << message; // Пример логирования в консоль
}

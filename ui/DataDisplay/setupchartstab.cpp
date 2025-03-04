#include "setupchartstab.h"
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QBarSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QCategoryAxis>
#include <QBarSet>
#include <QDateTimeAxis>
#include <QPieSeries>
#include <QScatterSeries>
#include <QSplineSeries>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QSpinBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QColorDialog>
#include <QFileDialog>
#include <QInputDialog>
#include <QPainter>
#include <QPrinter>
#include <QDebug>
#include <QPropertyAnimation>
#include <QBarCategoryAxis>

using namespace QtCharts;

setupChartsTab::setupChartsTab(DatabaseManager *db,Logger *logger, QWidget *parent) : QWidget(parent),dbManager(db),m_logger(logger) {
    m_logger->log(Logger::Info, "Инициализация setupChartsTab...");
    dbManager = db;
    setupUI();
}

void setupChartsTab::setupUI() {
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // Создание графика
    chart = new QChart();
    chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    mainLayout->addWidget(chartView, 4);

    // Панель параметров
    auto *parameterPanel = new QWidget(this);
    auto *parameterLayout = new QVBoxLayout(parameterPanel);
    parameterLayout->setContentsMargins(0, 0, 0, 0);

    // Верхняя строка: Выбор графика и типа
    auto *topRow = new QHBoxLayout();
    graphTypeSelector = new QComboBox(this);
    graphTypeSelector->addItems({"Линейный", "Spline", "Точечный", "Круговой"});
    topRow->addWidget(createLabel("Тип графика:"));
    topRow->addWidget(graphTypeSelector);
    parameterLayout->addLayout(topRow);

    auto *dataRow = new QHBoxLayout();
    graphSelector = new QComboBox(this);
    graphSelector->addItems({"Все","Широта", "Долгота", "Высота", "Скорость", "Курс","Широта по долготе","Высота по широте","Высота по долготе"});
    dataRow->addWidget(createLabel("Данные:"));
    dataRow->addWidget(graphSelector);
    parameterLayout->addLayout(dataRow);


    // Средняя строка: Фильтрация и сортировка
    auto *middleRow = new QHBoxLayout();

    // Фильтрация
    auto *filterGroup = new QGroupBox("Фильтрация", this);
    auto *filterLayout = new QVBoxLayout(filterGroup);
    filterComboBox = new QComboBox(this);
    filterComboBox->addItems({"ID", "Time", "Date", "Latitude", "Longitude", "Altitude", "Speed", "Course", "IsValid", "TimeStamp"});
    filterLayout->addWidget(createLabel("Поле фильтрации:"));
    filterLayout->addWidget(filterComboBox);
    filterLineEdit = new QLineEdit(this);
    filterLineEdit->setPlaceholderText("Введите значение для фильтрации");
    filterLayout->addWidget(createLabel("Значение:"));
    filterLayout->addWidget(filterLineEdit);
    middleRow->addWidget(filterGroup);

    // Сортировка
    auto *sortGroup = new QGroupBox("Сортировка", this);
    auto *sortLayout = new QVBoxLayout(sortGroup);
    sortComboBox = new QComboBox(this);
    sortComboBox->addItems({"ID", "Time", "Date", "Latitude", "Longitude", "Altitude", "Speed", "Course", "IsValid", "TimeStamp"});
    sortLayout->addWidget(createLabel("Поле сортировки:"));
    sortLayout->addWidget(sortComboBox);
    orderComboBox = new QComboBox(this);
    orderComboBox->addItems({"По возрастанию", "По убыванию"});
    sortLayout->addWidget(createLabel("Порядок:"));
    sortLayout->addWidget(orderComboBox);
    middleRow->addWidget(sortGroup);

    parameterLayout->addLayout(middleRow);

    // Нижняя строка: Настройки и кнопки
    auto *bottomRow = new QHBoxLayout();

    // Настройки графика
    auto *settingsGroup = new QGroupBox("Настройки", this);
    auto *settingsLayout = new QHBoxLayout(settingsGroup);
    densitySpinBox = new QSpinBox(this);
    densitySpinBox->setRange(1, 100);
    densitySpinBox->setValue(5);
    settingsLayout->addWidget(createLabel("Плотность:"));
    settingsLayout->addWidget(densitySpinBox);
    // В setupUI()
    auto *normComboBox = new QComboBox(this);
    normComboBox->addItems({"Без нормализации", "Минимум-Максимум", "Z-оценка"});
    settingsLayout->addWidget(createLabel("Нормализация:"));
    settingsLayout->addWidget(normComboBox);
    auto *colorButton = createColorButton();
    settingsLayout->addWidget(colorButton);

    borderCheckBox = new QCheckBox("Обводка", this);
    settingsLayout->addWidget(borderCheckBox);
    bottomRow->addWidget(settingsGroup);

    // Кнопки
    auto *buttonGroup = new QGroupBox("Действия", this);
    auto *buttonLayout = new QHBoxLayout(buttonGroup);
    flightComboBox = new QComboBox(this);
    buttonLayout->addWidget(flightComboBox);

    auto *loadButton = new QPushButton("Загрузить", this);
    buttonLayout->addWidget(loadButton);

    auto *resetButton = new QPushButton("Сброс", this);
    buttonLayout->addWidget(resetButton);

    auto *saveButton = new QPushButton("Сохранить", this);
    buttonLayout->addWidget(saveButton);

    bottomRow->addWidget(buttonGroup);
    parameterLayout->addLayout(bottomRow);

    mainLayout->addWidget(parameterPanel);

    // Добавление иконок к кнопкам
    saveButton->setIcon(QIcon(":/icons/save.svg"));
    loadButton->setIcon(QIcon(":/icons/refresh.svg"));
    resetButton->setIcon(QIcon(":/icons/clear.svg"));

    // Подключение сигналов
    connect(densitySpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &setupChartsTab::updatePointDensity);
    connect(colorButton, &QPushButton::clicked, this, &setupChartsTab::selectGraphColor);
    connect(borderCheckBox, &QCheckBox::clicked, this, &setupChartsTab::selectGraphColor);
    connect(saveButton, &QPushButton::clicked, this, &setupChartsTab::saveGraphsToFile);
    connect(loadButton, &QPushButton::clicked, this, &setupChartsTab::applyFilter);
    connect(resetButton, &QPushButton::clicked, this, &setupChartsTab::resetFilters);


    // Заполнение списка полетов
    QList<QString> flights = dbManager->getAllFlights();
    for (const QString &flight : flights) {
        flightComboBox->addItem(flight);
    }
    m_logger->log(Logger::Info, "Интерфейс графиков успешно настроен, количество полетов: " + QString::number(flights.size()));
}

QLabel* setupChartsTab::createLabel(const QString &text) {
    auto *label = new QLabel(text, this);
    label->setStyleSheet("font-weight: bold;"); // Установка жирного шрифта для меток
    return label;
}

QSpinBox* setupChartsTab::createSizeSpinBox() {
    auto *sizeSpinBox = new QSpinBox(this);
    sizeSpinBox->setRange(100, 800); // Установите диапазон значений для размера графика
    sizeSpinBox->setValue(400); // Установите значение по умолчанию
    return sizeSpinBox;
}

QPushButton* setupChartsTab::createColorButton() {
    auto *colorButton = new QPushButton("Выбрать цвет", this);
    return colorButton;
}

double setupChartsTab::normalize(double value, double min, double max, int method) {
    switch (method) {
    case 1: return (value - min) / (max - min); // Min-Max
    case 2: return (value - mean) / stddev; // Z-score
    default: return value; // Без нормализации
    }
}

void setupChartsTab::updateGraphSelection() {
    applyFilter(); // Обновляем графики с учетом новых данных графика
}

void setupChartsTab::selectGraphColor() {
    // Открываем диалог выбора цвета
    QColor selectedColor = QColorDialog::getColor(color, this, "Выберите цвет графика");

    // Проверяем, был ли выбран валидный цвет
    if (selectedColor.isValid()) {
        // Проверяем, есть ли график
        if (!chart->series().isEmpty()) {
            // Проверяем активирован ли borderCheckBox
            if (borderCheckBox->isChecked()) {
                // Устанавливаем цвет обводки графика
                // В QCharts нет прямой обводки, но можно изменить цвет линии
                QPen pen(selectedColor);
                pen.setWidth(2);
                dynamic_cast<QLineSeries*>(chart->series().first())->setPen(pen);
            } else {
                // Устанавливаем цвет графика
                dynamic_cast<QLineSeries*>(chart->series().first())->setPen(QPen(selectedColor));
            }
            color = selectedColor; // Сохраняем выбранный цвет
            chart->update(); // Обновляем график
        } else {
            logMessage("График пуст, цвет не может быть изменен.");
        }
    }
}

void setupChartsTab::updatePointDensity(int density) {
    m_logger->log(Logger::Debug, "Изменение плотности точек: " + QString::number(density));
    this->density = density; // Сохраняем значение плотности
    applyFilter(); // Обновляем графики с учетом новой плотности
}

void setupChartsTab::updateChartSize(int value) {
    chartView->setMinimumSize(value, value); // Устанавливаем минимальный размер графика
    chartView->setMaximumSize(value, value); // Устанавливаем максимальный размер графика
}

void setupChartsTab::resetFilters() {
    filterLineEdit->clear(); // Очищаем поле фильтрации
    sortComboBox->setCurrentIndex(0); // Сбрасываем сортировку на первое поле
    filterComboBox->setCurrentIndex(0); // Сбрасываем фильтрацию на первое поле
    orderComboBox->setCurrentIndex(0); // Сбрасываем направление сортировки на "По возрастанию"
    applyFilter(); // Обновляем данные без фильтрации
}

void setupChartsTab::applyFilter() {
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
    sortOrder = (sortOrder == "По возрастанию") ? "ASC" : "DESC";

    // Получаем отфильтрованные данные
    QList<NavigationData> Data = dbManager->getNavigationDataFilter(filterField, filterValue, sortField, sortOrder, flightName);
    m_logger->log(Logger::Info, QString("получено %1 записей").arg(Data.size()));
    // Обновляем график с новыми данными
    updateCharts(Data);
}

void setupChartsTab::setupCustomPlot(const QString &title, const QString &xTitle, const QString &yTitle, QVector<double> xData, QVector<double> yData) {
    // Очистка предыдущих данных
    chart->removeAllSeries();
    QList<QAbstractAxis*> axes = chart->axes();
    for (QAbstractAxis* axis : axes) {
        chart->removeAxis(axis);
        delete axis;
    }
    chart->setTitle(""); // Сброс заголовка

    bool isTimeBased = (xTitle == "Дата и время");

    chart->setTitle(title);

    // Определяем тип графика
    int graphType = graphTypeSelector->currentIndex();
    QAbstractSeries *abstractSeries = nullptr;

    switch (graphType) {
    case 0: {
        QLineSeries *series = new QLineSeries();
        series->setName(title);

        // Стиль линии
        QPen pen(color, 2);
        pen.setCapStyle(Qt::RoundCap);
        pen.setJoinStyle(Qt::RoundJoin);
        series->setPen(pen);

        // Заполнение данных
        for (int i = 0; i < xData.size(); ++i) {
            series->append(xData[i], yData[i]);
        }

        // Тултипы
        connect(series, &QLineSeries::hovered, [=](const QPointF &point, bool state) {
            if (state) {
                QString xStr = (xTitle == "Дата и время")
                                   ? QDateTime::fromMSecsSinceEpoch(point.x()).toString("dd.MM.yyyy HH:mm")
                                   : QString::number(point.x(), 'f', 4);

                QToolTip::showText(QCursor::pos(),
                                   QString("<b>%1</b><br>X: %2<br>Y: %3")
                                       .arg(title)
                                       .arg(xStr)
                                       .arg(point.y(), 0, 'f', 2),
                                   nullptr, {}, 1000);
            }
        });

        abstractSeries = series;
        break;
    }

    // Spline-график
    case 1: {
        QSplineSeries *series = new QSplineSeries();
        series->setName(title);

        // Стиль сглаженных линий
        QPen pen(color, 2);
        pen.setCapStyle(Qt::RoundCap);
        pen.setJoinStyle(Qt::RoundJoin);
        series->setPen(pen);

        // Оптимизация для большого объема данных
        const int maxPoints = 500;
        const double step = qMax(1.0, xData.size() / (double)maxPoints);

        // Децимация данных
        for (double i = 0; i < xData.size(); i += step) {
            int idx = qFloor(i);
            series->append(xData[idx], yData[idx]);
        }

        // Тултипы
        connect(series, &QSplineSeries::hovered, [=](const QPointF &point, bool state) {
            if (state) {
                QString xStr = (xTitle == "Дата и время")
                                   ? QDateTime::fromMSecsSinceEpoch(point.x()).toString("dd.MM.yyyh HH:mm")
                                   : QString::number(point.x(), 'f', 4);

                QToolTip::showText(QCursor::pos(),
                                   QString("<b>%1</b><br>X: %2<br>Y: %3")
                                       .arg(title)
                                       .arg(xStr)
                                       .arg(point.y(), 0, 'f', 2),
                                   nullptr, {}, 1000);
            }
        });

        abstractSeries = series;
        break;
    }
    case 2: { // Точечный график
        QXYSeries *series = nullptr;
        if (graphType == 0) series = new QLineSeries();
        else if (graphType == 1) series = new QSplineSeries();
        else series = new QScatterSeries();

        // Настройка стиля
        series->setPen(QPen(color, 2));
        if (graphType == 2) {
            dynamic_cast<QScatterSeries*>(series)->setMarkerSize(10);
            dynamic_cast<QScatterSeries*>(series)->setBorderColor(color.darker());
        }

        // Заполнение данных
        for (int i = 0; i < xData.size(); ++i) {
            series->append(xData[i], yData[i]);
        }

        // Настройка тултипов
        connect(series, &QXYSeries::hovered, [=](const QPointF &point, bool state) {
            if (state) {
                QString xValue;
                if (isTimeBased) {
                    xValue = QDateTime::fromMSecsSinceEpoch(point.x()).toString("dd.MM.yyyy HH:mm");
                } else {
                    xValue = QString::number(point.x(), 'f', 4);
                }

                QToolTip::showText(QCursor::pos(),
                                   QString("%1\nX: %2\nY: %3").arg(title).arg(xValue).arg(point.y(), 0, 'f', 2),
                                   nullptr, {}, 1000);
            }
        });

        abstractSeries = series;
        break;
    }
    case 3: { // Круговой график
        QPieSeries *series = new QPieSeries();
        int validCount = 0, invalidCount = 0;

        for (const auto &data : gnrmc) {
            data.isValid ? validCount++ : invalidCount++;
        }

        QPieSlice *validSlice = series->append("Валидные", validCount);
        QPieSlice *invalidSlice = series->append("Невалидные", invalidCount);

        // Настройка стиля
        validSlice->setColor(QColor("#4CAF50"));
        invalidSlice->setColor(QColor("#F44336"));
        validSlice->setLabelVisible(true);
        invalidSlice->setLabelVisible(true);

        // Тултипы для секторов
        connect(series, &QPieSeries::hovered, [=](QPieSlice *slice, bool state) {
            if (state) {
                double percent = slice->percentage() * 100;
                QToolTip::showText(QCursor::pos(),
                                   QString("%1\n%2 (%3%)").arg(title).arg(slice->label()).arg(percent, 0, 'f', 1),
                                   nullptr, {}, 800);
            }
        });

        abstractSeries = series;
        break;
    }
    default: return;
    }

    // Добавление серии и осей
    chart->addSeries(abstractSeries);
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);

    // Настройка осей
    if (graphType != 3) { // Не для круговой диаграммы
        QAbstractAxis *axisX = nullptr;
        QAbstractAxis *axisY = new QValueAxis();

        if (isTimeBased) {
            QDateTimeAxis *dtAxis = new QDateTimeAxis();
            dtAxis->setFormat("dd.MM.yyyy HH:mm");
            dtAxis->setRange(
                QDateTime::fromMSecsSinceEpoch(xData.first()),
                QDateTime::fromMSecsSinceEpoch(xData.last())
                );
            axisX = dtAxis;
        } else {
            QValueAxis *valAxis = new QValueAxis();
            valAxis->setRange(
                *std::min_element(xData.begin(), xData.end()),
                *std::max_element(xData.begin(), xData.end())
                );
            axisX = valAxis;
        }

        axisX->setTitleText(xTitle);
        axisY->setTitleText(yTitle);
        axisY->setRange(
            *std::min_element(yData.begin(), yData.end()),
            *std::max_element(yData.begin(), yData.end())
            );

        chart->addAxis(axisX, Qt::AlignBottom);
        chart->addAxis(axisY, Qt::AlignLeft);
        abstractSeries->attachAxis(axisX);
        abstractSeries->attachAxis(axisY);
    }

    // Анимация
    QPropertyAnimation *animation = new QPropertyAnimation(chart, "opacity");
    animation->setDuration(800);
    animation->setStartValue(0);
    animation->setEndValue(1);
    animation->start(QAbstractAnimation::DeleteWhenStopped);

    // Общее оформление
    chart->setBackgroundBrush(QBrush(QColor("#F5F5F5")));
    chart->setTitleFont(QFont("Arial", 12, QFont::Bold));
    chart->setTitleBrush(QBrush(Qt::darkBlue));
    chartView->setRenderHint(QPainter::Antialiasing);
}

void setupChartsTab::setupAxes(QAbstractSeries *series, const QVector<double> &xData, const QVector<double> &yData, const QString &xTitle, const QString &yTitle) {
    int currentIndex = graphSelector->currentIndex();
    if (currentIndex >= 0 && currentIndex < 5) {
        // Установка осей для временных данных
        QDateTimeAxis *axisX = new QDateTimeAxis;
        axisX->setFormat("dd.MM.yyyy HH:mm"); // Формат отображения даты и времени
        axisX->setTitleText(xTitle);
        chart->addAxis(axisX, Qt::AlignBottom);
        series->attachAxis(axisX);

        QValueAxis *axisY = new QValueAxis;
        axisY->setTitleText(yTitle);
        chart->addAxis(axisY, Qt::AlignLeft);
        series->attachAxis(axisY);

        // Устанавливаем диапазон осей
        axisX->setRange(QDateTime::fromMSecsSinceEpoch(xData.first()), QDateTime::fromMSecsSinceEpoch(xData.last()));
        axisY->setRange(*std::min_element(yData.begin(), yData.end()), *std::max_element(yData.begin(), yData.end()));
    } else {
        // Создаем оси по умолчанию
        chart->createDefaultAxes();
        chart->axes(Qt::Horizontal).first()->setTitleText(xTitle);
        chart->axes(Qt::Vertical).first()->setTitleText(yTitle);

        // Устанавливаем диапазон осей
        chart->axes(Qt::Horizontal).first()->setRange(*std::min_element(xData.begin(), xData.end()),
                                                      *std::max_element(xData.begin(), xData.end()));
        chart->axes(Qt::Vertical).first()->setRange(*std::min_element(yData.begin(), yData.end()),
                                                    *std::max_element(yData.begin(), yData.end()));
    }
}

void setupChartsTab::updateCharts(const QList<NavigationData> &navigationDataList) {
    m_logger->log(Logger::Info, "Обновление графиков...");

    if (navigationDataList.isEmpty()) {
        m_logger->log(Logger::Warning, "Нет данных для обновления графиков.");
        return;
    }

    m_logger->log(Logger::Info, QString("Получено данных для обновления графиков: %1").arg(navigationDataList.size()));
    try{
    QVector<double> latitudeData, longitudeData, altitudeData, timeData, speedData, courseData;

    for (int i = 0; i < navigationDataList.count(); i++) {
        if (i % density == 0) { // Используем плотность для выборки данных
            NavigationData data = navigationDataList[i];

            GNRMCData gnrm;
            GNGGAData gngga;
            GNZDAData gnzda;

            QDataStream stream(&data.data, QIODevice::ReadOnly);
            stream >> data.id
                     >> data.timestamp
                     >> gnzda.date
                     >> gnzda.time
                     >> gnrm.isValid
                     >> gngga.altitude
                     >> gnrm.latitude
                     >> gnrm.longitude
                     >> gnrm.speed
                     >> gnrm.course;

            // Проверяем валидность данных
            if (gnrm.isValid && gngga.altitude > 0) {
                QDateTime dateTime(gnzda.date, gnzda.time);
                timeData.append(dateTime.toMSecsSinceEpoch());
                latitudeData.append(gnrm.latitude);
                longitudeData.append(gnrm.longitude);
                altitudeData.append(gngga.altitude);
                speedData.append(gnrm.speed);
                courseData.append(gnrm.course);
            }
            gnrmc.append(gnrm);
        }
    }

    if (!latitudeData.isEmpty() && !longitudeData.isEmpty() && !altitudeData.isEmpty() && !timeData.isEmpty()) {
        // Устанавливаем данные в график в зависимости от выбранного типа
        int currentIndex = graphSelector->currentIndex();
        if (currentIndex == 0) { // все данные
            // Очистка предыдущих данных
            chart->removeAllSeries();
            QList<QAbstractAxis*> axes = chart->axes();
            for (QAbstractAxis* axis : axes) {
                chart->removeAxis(axis);
                delete axis;
            }

            // Инициализация серий
            latSeries = new QLineSeries();
            lonSeries = new QLineSeries();
            altSeries = new QLineSeries();
            speedSeries = new QLineSeries();
            courseSeries = new QLineSeries();

            // Вычисление статистики
            minLat = *std::min_element(latitudeData.begin(), latitudeData.end());
            maxLat = *std::max_element(latitudeData.begin(), latitudeData.end());
            minLon = *std::min_element(longitudeData.begin(), longitudeData.end());
            maxLon = *std::max_element(longitudeData.begin(), longitudeData.end());
            minAlt = *std::min_element(altitudeData.begin(), altitudeData.end());
            maxAlt = *std::max_element(altitudeData.begin(), altitudeData.end());
            minSpeed = *std::min_element(speedData.begin(), speedData.end());
            maxSpeed = *std::max_element(speedData.begin(), speedData.end());
            minCourse = *std::min_element(courseData.begin(), courseData.end());
            maxCourse = *std::max_element(courseData.begin(), courseData.end());

            // Заполнение серий с нормализацией
            for (int i = 0; i < timeData.size(); ++i) {
                double t = timeData[i];

                if (maxLat != minLat)
                    latSeries->append(t, (latitudeData[i] - minLat) / (maxLat - minLat));
                else
                    latSeries->append(t, 0.5);

                if (maxLon != minLon)
                    lonSeries->append(t, (longitudeData[i] - minLon) / (maxLon - minLon));
                else
                    lonSeries->append(t, 0.5);

                if (maxAlt != minAlt)
                    altSeries->append(t, (altitudeData[i] - minAlt) / (maxAlt - minAlt));
                else
                    altSeries->append(t, 0.5);

                if (maxSpeed != minSpeed)
                    speedSeries->append(t, (speedData[i] - minSpeed) / (maxSpeed - minSpeed));
                else
                    speedSeries->append(t, 0.5);

                if (maxCourse != minCourse)
                    courseSeries->append(t, (courseData[i] - minCourse) / (maxCourse - minCourse));
                else
                    courseSeries->append(t, 0.5);
            }

            // Настройка серий
            latSeries->setName("Широта");
            lonSeries->setName("Долгота");
            altSeries->setName("Высота");
            speedSeries->setName("Скорость");
            courseSeries->setName("Курс");

            // Создание осей
            QDateTimeAxis *axisX = new QDateTimeAxis();
            axisX->setFormat("dd.MM.yyyy HH:mm");
            axisX->setTitleText("Время");
            chart->addAxis(axisX, Qt::AlignBottom);

            QValueAxis *axisY = new QValueAxis();
            axisY->setTitleText("Нормализованные значения");
            axisY->setRange(0, 1);
            chart->addAxis(axisY, Qt::AlignLeft);

            // Добавление серий
            chart->addSeries(latSeries);
            chart->addSeries(lonSeries);
            chart->addSeries(altSeries);
            chart->addSeries(speedSeries);
            chart->addSeries(courseSeries);

            // Привязка осей
            latSeries->attachAxis(axisX);
            latSeries->attachAxis(axisY);
            lonSeries->attachAxis(axisX);
            lonSeries->attachAxis(axisY);
            altSeries->attachAxis(axisX);
            altSeries->attachAxis(axisY);
            speedSeries->attachAxis(axisX);
            speedSeries->attachAxis(axisY);
            courseSeries->attachAxis(axisX);
            courseSeries->attachAxis(axisY);

            // Настройка диапазона времени
            axisX->setRange(
                QDateTime::fromMSecsSinceEpoch(timeData.first()),
                QDateTime::fromMSecsSinceEpoch(timeData.last())
                );

            // Настройка легенды
            chart->legend()->setVisible(true);
            chart->legend()->setAlignment(Qt::AlignBottom);

            // Tooltips
            auto setupHover = [](QLineSeries* series, double min, double max, const QString& unit) {
                QObject::connect(series, &QLineSeries::hovered,
                                 [series, min, max, unit](const QPointF& point, bool state) {
                                     if (state) {
                                         double value = point.y() * (max - min) + min;
                                         QToolTip::showText(QCursor::pos(),
                                                            QString("%1: %2 %3").arg(series->name()).arg(value, 0, 'f', 4).arg(unit));
                                     }
                                 });
            };

            setupHover(latSeries, minLat, maxLat, "°");
            setupHover(lonSeries, minLon, maxLon, "°");
            setupHover(altSeries, minAlt, maxAlt, "m");
            setupHover(speedSeries, minSpeed, maxSpeed, "m/s");
            setupHover(courseSeries, minCourse, maxCourse, "°");
        }else if (currentIndex == 1) { // Широта
            setupCustomPlot("Широта", "Дата и время", "Широта", timeData, latitudeData);
        } else if (currentIndex == 2) { // Долгота
            setupCustomPlot("Долгота", "Дата и время", "Долгота", timeData, longitudeData);
        } else if (currentIndex == 3) { // Высота
            setupCustomPlot("Высота", "Дата и время", "Высота", timeData, altitudeData);
        } else if (currentIndex == 4) { // Скорость
            setupCustomPlot("Скорость", "Дата и время", "Скорость", timeData, speedData);
        } else if (currentIndex == 5) { // Курс
            setupCustomPlot("Курс", "Дата и время", "Курс", timeData, courseData);
        } else if (currentIndex == 6) { // Широта по долготе
            setupCustomPlot("Широта по долготе", "Долготе", "Широта", longitudeData, latitudeData);
        } else if (currentIndex == 7) { // Высота по широте
            setupCustomPlot("Высота по широте", "Широта", "Высота", latitudeData, altitudeData);
        } else if (currentIndex == 8) { // Высота по долготе
            setupCustomPlot("Высота по долготе", "Долготе", "Высота", altitudeData, longitudeData);
        }
    } else {
        m_logger->log(Logger::Info, QString("Недостаточно данных для обновления графика"));
    }
} catch (const std::exception &e) {
        m_logger->log(Logger::Error, QString("Ошибка при обновлении графика: %1").arg(e.what()));
    QMessageBox::critical(this, "Ошибка", "Не удалось получить данные для графиков.");
}
}

void setupChartsTab::saveGraphsToFile() {
    m_logger->log(Logger::Info, "Попытка сохранить графики в файл");
    QStringList formats = {"PNG", "JPEG", "BMP", "PDF"};
    bool ok;
    QString format = QInputDialog::getItem(this, "Выберите формат", "Формат сохранения:", formats, 0, false, &ok);

    if (ok && !format.isEmpty()) {
        QString fileName = QFileDialog::getSaveFileName(this, "Сохранить график как", "",
            QString("%1 Files (*.%1)").arg(format.toLower()));

        if (fileName.isEmpty()){
            m_logger->log(Logger::Warning, "Сохранение графика отменено пользователем.");
            return;
        }
        try{
        // Сохраняем график в файл в зависимости от выбранного формата
        if (format == "PNG") {
            chartView->grab().save(fileName, "PNG");
        } else if (format == "JPEG") {
            chartView->grab().save(fileName, "JPEG");
        } else if (format == "BMP") {
            chartView->grab().save(fileName, "BMP");
        } else if (format == "PDF") {
            // Сохранение в PDF требует дополнительной обработки
            // Для этого можно использовать QPrinter
            QPrinter printer(QPrinter::HighResolution);
            printer.setOutputFormat(QPrinter::PdfFormat);
            printer.setOutputFileName(fileName);
            printer.setPageSize(QPrinter::A4); // Установите размер страницы, если необходимо

            QPainter painter(&printer); // Создаем QPainter, связанный с QPrinter
            chartView->render(&painter); // Рендерим график на QPainter
        }

        m_logger->log(Logger::Info, QString("Графики успешно сохранены в файл: %1").arg(fileName));
        } catch (const std::exception &e) {
                m_logger->log(Logger::Error, QString("Ошибка при сохранении графиков: %1").arg(e.what()));
            QMessageBox::critical(this, "Ошибка", "Не удалось сохранить графики.");
        }
    }
}

void setupChartsTab::logMessage(const QString &message) {
    qDebug() << message; // Пример логирования в консоль
}

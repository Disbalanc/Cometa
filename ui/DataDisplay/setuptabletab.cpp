#include "setuptabletab.h"
#include "tableconfigdialog.h"
#include <QHBoxLayout>
#include <QFrame>
#include <QIcon>
#include <QGroupBox>
#include <QFormLayout>

setupTableTab::setupTableTab(DatabaseManager *db,Logger *logger, QWidget *parent) : QWidget(parent),dbManager(db),m_logger(logger) {
    m_logger->log(Logger::Info, "Инициализация вкладки таблицы...");
    setupUI();
}

void setupTableTab::setupUI() {
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(5, 5, 5, 5);
    mainLayout->setSpacing(5);

    // Таблица данных
    dataTable = new QTableWidget(this);
    dataTable->setColumnCount(11);
    dataTable->setMinimumSize(400, 500);
    dataTable->setHorizontalHeaderLabels({"№", "ID", "Time", "Date", "Latitude", "Longitude", "Altitude", "Speed", "Course", "IsValid", "TimeStamp"});
    dataTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    dataTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    mainLayout->addWidget(dataTable, 1); // 70% высоты

    // Панель управления
    QWidget *controlPanel = new QWidget(this);
    QVBoxLayout *controlLayout = new QVBoxLayout(controlPanel);
    controlLayout->setContentsMargins(0, 0, 0, 0);

    // Группа фильтров и сортировки
    QGroupBox *filterSortGroup = new QGroupBox("Фильтры и сортировка", this);
    QHBoxLayout *filterSortLayout = new QHBoxLayout(filterSortGroup);

    // Фильтрация
    QGroupBox *filterGroup = new QGroupBox("Фильтрация", this);
    QFormLayout *filterForm = new QFormLayout(filterGroup);
    filterComboBox = new QComboBox(this);
    filterComboBox->addItems({"ID", "Time", "Date", "Latitude", "Longitude", "Altitude", "Speed", "Course", "IsValid", "TimeStamp"});
    filterLineEdit = new QLineEdit(this);
    filterForm->addRow("Поле:", filterComboBox);
    filterForm->addRow("Значение:", filterLineEdit);
    filterSortLayout->addWidget(filterGroup);

    // Сортировка
    QGroupBox *sortGroup = new QGroupBox("Сортировка", this);
    QFormLayout *sortForm = new QFormLayout(sortGroup);
    sortComboBox = new QComboBox(this);
    sortComboBox->addItems({"ID", "Time", "Date", "Latitude", "Longitude", "Altitude", "Speed", "Course", "IsValid", "TimeStamp"});
    orderComboBox = new QComboBox(this);
    orderComboBox->addItems({"По возрастанию", "По убыванию"});
    sortForm->addRow("Сортировать по:", sortComboBox);
    sortForm->addRow("Порядок:", orderComboBox);
    filterSortLayout->addWidget(sortGroup);

    controlLayout->addWidget(filterSortGroup);

    // Группа управления полетами
    QGroupBox *flightGroup = new QGroupBox("Управление полетами", this);
    QHBoxLayout *flightLayout = new QHBoxLayout(flightGroup);
    flightComboBox = new QComboBox(this);
    flightLayout->addWidget(new QLabel("Полет:", this));
    flightLayout->addWidget(flightComboBox, 1);

    auto *loadButton = new QPushButton("Загрузить", this);
    flightLayout->addWidget(loadButton);
    connect(loadButton, &QPushButton::clicked, this, &setupTableTab::applyFilter);

    QPushButton *deleteFlightButton = new QPushButton("Удалить полет", this);
    deleteFlightButton->setStyleSheet("background-color: #ff4444; color: white;");
    flightLayout->addWidget(deleteFlightButton);

    QPushButton *deleteRowsButton = new QPushButton("Удалить строки", this);
    deleteRowsButton->setStyleSheet("background-color: #ff4444; color: white;");
    flightLayout->addWidget(deleteRowsButton);

    controlLayout->addWidget(flightGroup);

    // Кнопки действий
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *applyButton = new QPushButton("Применить фильтры", this);
    QPushButton *resetButton = new QPushButton("Сбросить", this);
    QPushButton *saveButton = new QPushButton("Сохранить", this);
    // В setupUI добавить кнопку настройки:
    QPushButton *configButton = new QPushButton("Настроить таблицу", this);

    buttonLayout->addWidget(configButton);
    buttonLayout->addWidget(applyButton);
    buttonLayout->addWidget(resetButton);
    buttonLayout->addWidget(saveButton);
    controlLayout->addLayout(buttonLayout);

    mainLayout->addWidget(controlPanel); // 30% высоты

    // Подключение сигналов
    connect(configButton, &QPushButton::clicked, this, &setupTableTab::configureTable);
    connect(applyButton, &QPushButton::clicked, this, &setupTableTab::applyFilter);
    connect(resetButton, &QPushButton::clicked, this, &setupTableTab::resetFilters);
    connect(saveButton, &QPushButton::clicked, this, &setupTableTab::saveData);
    connect(deleteRowsButton, &QPushButton::clicked, this, &setupTableTab::deleteSelectedRows);
    connect(deleteFlightButton, &QPushButton::clicked, this, &setupTableTab::deleteCurrentFlight);

    // Загрузка списка полетов
    loadFlights();
}

void setupTableTab::loadFlights() {
    flightComboBox->clear();
    QList<QString> flights = dbManager->getAllFlights(); // Получаем список полетов

    for (const QString &flight : flights) {
        flightComboBox->addItem(flight); // Добавляем каждый полет в комбобокс
    }
    m_logger->log(Logger::Info, "Интерфейс Таблицы успешно настроен, количество полетов: " + QString::number(flights.size()));
}

void setupTableTab::resetFilters() {
    m_logger->log(Logger::Info, "Сброс фильтров и сортировки");
    filterLineEdit->clear(); // Очищаем поле фильтрации
    sortComboBox->setCurrentIndex(0); // Сбрасываем сортировку на первое поле
    filterComboBox->setCurrentIndex(0); // Сбрасываем фильтрацию на первое поле
    orderComboBox->setCurrentIndex(0); // Сбрасываем направление сортировки на "По возрастанию"

    // Вызываем метод для обновления данных без фильтрации
    applyFilter();
}

void setupTableTab::applyFilter() {
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

    // Преобразуем направление сортировки в SQL-формат
    sortOrder = (sortOrder == "По возрастанию") ? "ASC" : "DESC"; // Преобразуем в ASC или DESC

    // Получаем отфильтрованные данные
    QList<NavigationData> filteredData = dbManager->getNavigationDataFilter(filterField, filterValue, sortField, sortOrder, flightName);

    // Обновляем таблицу с новыми данными
    updateDataTable(filteredData);
}

void setupTableTab::updateDataTable(const QList<NavigationData> &navigationDataList) {
    m_logger->log(Logger::Debug,
                  QString("Обновление таблицы данными (%1 записей)").arg(navigationDataList.size()));
    try{
    dataTable->setRowCount(navigationDataList.size());
    for (int row = 0; row < navigationDataList.size(); ++row) {
        NavigationData data = navigationDataList.at(row);
        populateRow(row, data);
    }}catch(const std::exception& e) {
            m_logger->log(Logger::Error,
                          QString("Ошибка при обновлении данных таблицы: %1").arg(e.what()));
    }
}

void setupTableTab::updateDataTableCustom(const QList<NavigationDataTable> &navigationDataList) {
    m_logger->log(Logger::Debug,
                  QString("Обновление таблицы данными (%1 записей)").arg(navigationDataList.size()));
    try{
        dataTable->setRowCount(navigationDataList.size());
        for (int row = 0; row < navigationDataList.size(); ++row) {
            NavigationDataTable data = navigationDataList.at(row);
            populateRowCustom(row, data);
        }}catch(const std::exception& e) {
        m_logger->log(Logger::Error,
                      QString("Ошибка при обновлении данных таблицы: %1").arg(e.what()));
    }
}

void setupTableTab::populateRowCustom(int row, NavigationDataTable &data) {
    int column = 0;

    // Очищаем строку перед заполнением
    dataTable->setRowCount(row + 1);

    // Перебираем все выбранные поля в порядке их отображения
    for (const QString &field : selectedFields) {
        QVariant value = data.customData.value(field);
        QString displayText;

        // Специальное форматирование для определенных типов данных
        if (field.endsWith(".time")) {
            QTime time = QTime::fromString(value.toString(), "hh:mm:ss.zzz");
            displayText = time.toString("hh:mm:ss");
        }
        else if (field.endsWith(".date")) {
            QDate date = QDate::fromString(value.toString(), "yyyy-MM-dd");
            displayText = date.toString("dd.MM.yyyy");
        }
        else if (field.endsWith(".isValid") || field.endsWith(".statusNav")) {
            displayText = value.toBool() ? "Да" : "Нет";
        }
        else {
            displayText = value.toString();
        }

        // Создаем элемент таблицы
        QTableWidgetItem *item = new QTableWidgetItem(displayText);

        // Выравнивание для числовых полей
        if (field.contains("latitude", Qt::CaseInsensitive) ||
            field.contains("longitude", Qt::CaseInsensitive) ||
            field.contains("altitude", Qt::CaseInsensitive) ||
            field.contains("speed", Qt::CaseInsensitive) ||
            field.contains("course", Qt::CaseInsensitive)) {
            item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        }

        dataTable->setItem(row, column++, item);
    }

    // Добавляем служебные поля (если нужно)
    dataTable->setItem(row, column++, new QTableWidgetItem(QString::number(row + 1))); // №
    dataTable->setItem(row, column++, new QTableWidgetItem(QString::number(data.id))); // ID
    dataTable->setItem(row, column, new QTableWidgetItem(data.timestamp.toString("yyyy-MM-dd hh:mm:ss"))); // Timestamp
}

void setupTableTab::populateRow(int row, NavigationData &data) {
    GNRMCData gnrmc;
    GNZDAData gnzda;
    GNGGAData gngga;
    QDataStream stream(data.data);
    stream >> data.id >> data.timestamp >> gnzda.date >> gnzda.time >> gnrmc.isValid >> gngga.altitude
                      >> gnrmc.latitude >> gnrmc.longitude >> gnrmc.speed >> gnrmc.course;

    dataTable->setItem(row, 0, new QTableWidgetItem(QString::number(row + 1))); // Индекс строки
    dataTable->setItem(row, 1, new QTableWidgetItem(QString::number(data.id))); // ID
    dataTable->setItem(row, 10, new QTableWidgetItem(data.timestamp.toString("yyyy-MM-dd hh:mm:ss"))); // Время парсинга
    dataTable->setItem(row, 4, new QTableWidgetItem(QString::number(gnrmc.latitude))); // Широта
    dataTable->setItem(row, 5, new QTableWidgetItem(QString::number(gnrmc.longitude))); // Долгота
    dataTable->setItem(row, 7, new QTableWidgetItem(QString::number(gnrmc.speed))); // Скорость
    dataTable->setItem(row, 8, new QTableWidgetItem(QString::number(gnrmc.course))); // Курс
    dataTable->setItem(row, 9, new QTableWidgetItem(gnrmc.isValid ? "Да" : "Нет")); // Валидность
    dataTable->setItem(row, 2, new QTableWidgetItem(gnzda.time.toString("hh:mm:ss"))); // Время GNZDA
    dataTable->setItem(row, 3, new QTableWidgetItem(gnzda.date.toString("yyyy-MM-dd"))); // Дата GNZDA
    dataTable->setItem(row, 6, new QTableWidgetItem(QString::number(gngga.altitude))); // Высота
}

void setupTableTab::configureTable() {
    TableConfigDialog dialog(dbManager->getTablesStructure(), this);
    if (dialog.exec() == QDialog::Accepted) {
        selectedFields.clear();
        columnAliases.clear();

        auto fields = dialog.getSelectedFields();
        foreach (const auto& field, fields) {
            selectedFields.append(field.first);
            columnAliases.insert(field.first, field.second);
        }

        updateTableHeaders();
        applyFilterCustom();
    }
}

void setupTableTab::updateTableHeaders() {
    dataTable->setColumnCount(selectedFields.size());
    QStringList headers;
    foreach (const QString& field, selectedFields) {
        headers << columnAliases.value(field, field);
    }
    dataTable->setHorizontalHeaderLabels(headers);
}

void setupTableTab::applyFilterCustom() {
    // Обновленный запрос данных
    QList<NavigationDataTable> filteredData = dbManager->getCustomData(
        selectedFields,
        filterComboBox->currentText(),
        filterLineEdit->text(),
        sortComboBox->currentText(),
        orderComboBox->currentText(),
        flightComboBox->currentText()
        );

    updateDataTableCustom(filteredData);
}

void setupTableTab::deleteCurrentFlight() {
    QString flightName = flightComboBox->currentText();
    if (flightName.isEmpty()) {
        m_logger->log(Logger::Warning, "Попытка удаления пустого полета");
        return;
    }

    m_logger->log(Logger::Info,
                  QString("Попытка удаления полета: %1").arg(flightName));

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Подтверждение удаления", "Вы уверены, что хотите удалить выбранный полет?",
                                  QMessageBox::Yes | QMessageBox::No);
    if (reply != QMessageBox::Yes) {
        m_logger->log(Logger::Debug, "Удаление отменено пользователем");
        return;
    }

    if (dbManager->deleteFlight(flightName)) {
        // Обновить список полетов
        loadFlights();
        m_logger->log(Logger::Info, "Полет успешно удален");
    } else {
        m_logger->log(Logger::Error, "Ошибка удаления полета");
    }
}

void setupTableTab::deleteSelectedRows() {
    QItemSelectionModel *selectionModel = dataTable->selectionModel();
    QModelIndexList selectedRows = selectionModel->selectedRows();

    if (selectedRows.isEmpty()) {
        m_logger->log(Logger::Warning, "Попытка удаления без выбранных строк");
        return;
    }

    m_logger->log(Logger::Info,
                  QString("Запрошено удаление %1 строк").arg(selectedRows.size()));

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Подтверждение удаления", "Вы уверены, что хотите удалить выбранные строки?",
                                  QMessageBox::Yes | QMessageBox::No);
    if (reply != QMessageBox::Yes) {
        m_logger->log(Logger::Debug, "Удаление строк отменено пользователем");
        return;
    }
    int successCount = 0;
    for (const QModelIndex &index : selectedRows) {
        int row = index.row();
        int id = dataTable->item(row, 1)->text().toInt(); // Изменено на 1 для получения ID

        if (dbManager->deleteNavigationDataById(id)) {
            successCount++;
        } else {
            m_logger->log(Logger::Error,
                          QString("Ошибка удаления записи ID: %1").arg(id));
        }
    }

    applyFilter(); // Обновляем таблицу после удаления
    m_logger->log(Logger::Info,
                  QString("Удалено %1 из %2 строк").arg(successCount).arg(selectedRows.size()));
}

void setupTableTab::saveData() {
    QStringList formats = {"CSV", "JSON", "XML"};
    bool ok;
    QString format = QInputDialog::getItem(this, "Выберите формат", "Формат сохранения:", formats, 0, false, &ok);
    if (ok && !format.isEmpty()) {
        m_logger->log(Logger::Info,
                      QString("Запрошено сохранение данных в формате: %1").arg(format));
        if (format == "CSV") {
            saveDataAsCSV();
        } else if (format == "JSON") {
            saveDataAsJSON();
        } else if (format == "XML") {
            saveDataAsXML();
        }
    }
}

void setupTableTab::saveDataAsCSV() {
    QString fileName = QFileDialog::getSaveFileName(this, "Сохранить как", "", "CSV Files (*.csv)");
    if (fileName.isEmpty()){
        m_logger->log(Logger::Debug, "Сохранение CSV отменено");
        return;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    m_logger->log(Logger::Error,
                QString("Ошибка сохранения CSV: %1").arg(file.errorString()));
        return;
    }

    QTextStream out(&file);
    for (int row = 0; row < dataTable->rowCount(); ++row) {
        QStringList rowData;
        for (int col = 0; col < dataTable->columnCount(); ++col) {
            rowData << dataTable->item(row, col)->text();
        }
        out << rowData.join(",") << "\n";
    }
    file.close();
    m_logger->log(Logger::Info,
                  QString("Данные сохранены в CSV: %1").arg(fileName));
}

void setupTableTab::saveDataAsJSON() {
    QString fileName = QFileDialog::getSaveFileName(this, "Сохранить как", "", "JSON Files (*.json)");
    if (fileName.isEmpty()){
        m_logger->log(Logger::Debug, "Сохранение JSON отменено");
        return;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        m_logger->log(Logger::Error,
                      QString("Ошибка сохранения JSON: %1").arg(file.errorString()));
        return;
    }

    QJsonArray jsonArray;
    for (int row = 0; row < dataTable->rowCount(); ++row) {
        QJsonObject jsonObject;
        for (int col = 0; col < dataTable->columnCount(); ++col) {
            jsonObject[dataTable->horizontalHeaderItem(col)->text()] = dataTable->item(row, col)->text();
        }
        jsonArray.append(jsonObject);
    }

    QJsonDocument jsonDoc(jsonArray);
    file.write(jsonDoc.toJson());
    file.close();
    m_logger->log(Logger::Info,
                  QString("Данные сохранены в JSON: %1").arg(fileName));
}

void setupTableTab::saveDataAsXML() {
    QString fileName = QFileDialog::getSaveFileName(this, "Сохранить как", "", "XML Files (*.xml)");
    if (fileName.isEmpty()){
        m_logger->log(Logger::Debug, "Сохранение XML отменено");
        return;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        m_logger->log(Logger::Error,
                      QString("Ошибка сохранения XML: %1").arg(file.errorString()));
        return;
    }

    QXmlStreamWriter xmlWriter(&file);
    xmlWriter.setAutoFormatting(true);
    xmlWriter.writeStartDocument();
    xmlWriter.writeStartElement("Data");

    for (int row = 0; row < dataTable->rowCount(); ++row) {
        xmlWriter.writeStartElement("Row");
        for (int col = 0; col < dataTable->columnCount(); ++col) {
            xmlWriter.writeTextElement(dataTable->horizontalHeaderItem(col)->text(), dataTable->item(row, col)->text());
        }
        xmlWriter.writeEndElement(); // Row
    }

    xmlWriter.writeEndElement(); // Data
    xmlWriter.writeEndDocument();
    file.close();
    m_logger->log(Logger::Info,
                  QString("Данные сохранены в XML: %1").arg(fileName));
}

QLabel* setupTableTab::createLabel(const QString &text) {
    return new QLabel(text, this);
}

void setupTableTab::logMessage(const QString &message) {
    // Логирование сообщений в файл или консоль
    qDebug() << message; // Пример логирования в консоль
}

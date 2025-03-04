#ifndef SETUPTABLETAB_H
#define SETUPTABLETAB_H

#include <QComboBox>
#include <QPushButton>
#include <QTableWidget>
#include <QWidget>
#include <QFile>
#include <QInputDialog>
#include <QFileDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QXmlStreamWriter>
#include <QHeaderView>
#include <QLabel>

#include "databasemanager.h"
#include "NavigationData.h"

class setupTableTab  : public QWidget
{
    Q_OBJECT
public:
    setupTableTab(DatabaseManager *db,Logger *logger,QWidget *parent = nullptr);
    void updateDataTable(const QList<NavigationData> &navigationDataList);
    void updateDataTableCustom(const QList<NavigationDataTable> &navigationDataList);
    void saveData(); // Метод для сохранения данных в файл
    void resetFilters();

private slots:
    void applyFilter(); // Слот для применения фильтра
    void deleteCurrentFlight();
    void configureTable();

private:
    void applyFilterCustom();
    void updateTableHeaders();

    QMap<QString, QString> columnAliases; // Ключ: table.field, Значение: псевдоним
    QStringList selectedFields; // Список выбранных полей в формате "table.field"

    void setupUI();
    void saveDataAsCSV(); // Метод для сохранения данных в CSV
    void saveDataAsJSON(); // Метод для сохранения данных в JSON
    void saveDataAsXML(); // Метод для сохранения данных в XML
    void loadFlightData(const QString &flightName);
    void populateRow(int row, NavigationData &data);
    void populateRowCustom(int row, NavigationDataTable &data);

    void loadTableSelection();
    void loadFlights();
    void deleteSelectedRows();
    void logMessage(const QString &message);

    Logger *m_logger;
    DatabaseManager *dbManager; // Менеджер базы данных

    QLabel *createLabel(const QString &text);

    QTableWidget *dataTable; // Таблица для отображения данных
    QComboBox *flightComboBox; // Комбобокс для выбора полета
    QPushButton *loadFlightButton; // Кнопка для загрузки данных
    QLineEdit *filterLineEdit; // Поле для ввода фильтра
    QComboBox *filterComboBox; // Выпадающий список для выбора поля фильтрации
    QComboBox *sortComboBox; // Выпадающий список для выбора поля сортировки
    QComboBox *orderComboBox; // Выпадающий список для выбора порядка сортировки
    QPushButton *filterButton; // Кнопка для применения фильтра

};

#endif // SETUPTABLETAB_H

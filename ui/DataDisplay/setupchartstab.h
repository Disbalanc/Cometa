#ifndef SETUPCHARTSTAB_H
#define SETUPCHARTSTAB_H

#include <QWidget>
#include <QChartView>
#include <QtCharts/QLineSeries>
#include <QChartView>
#include "qcustomplot.h"
#include "databasemanager.h"
#include "NavigationData.h"

using namespace QtCharts;

class setupChartsTab : public QWidget {
    Q_OBJECT

public:
    explicit setupChartsTab(DatabaseManager *db,Logger *logger,QWidget *parent = nullptr); // Конструктор с указателем на родительский класс
    void updateCharts(const QList<NavigationData> &navigationDataList);
    QCustomPlot *getChartPlot() const; // Метод для получения указателя на график
    void resetFilters();

private slots:
    void applyFilter(); // Слот для применения фильтра
    void updateGraphSelection();

private:
    // Добавляем члены для хранения серий
    QLineSeries *latSeries = nullptr;
    QLineSeries *lonSeries = nullptr;
    QLineSeries *altSeries = nullptr;
    QLineSeries *speedSeries = nullptr;
    QLineSeries *courseSeries = nullptr;



    // Добавляем члены для статистических данных
    double minLat = 0, maxLat = 0;
    double minLon = 0, maxLon = 0;
    double minAlt = 0, maxAlt = 0;
    double minSpeed = 0, maxSpeed = 0;
    double minCourse = 0, maxCourse = 0;

    double mean = 0, stddev = 0;
    double normalize(double value, double min, double max, int method);
    void setupUI();
    void loadGraphSelection();
    void loadFlightData();
    void setupCustomPlot(const QString &title, const QString &xTitle, const QString &yTitle, QVector<double> xData,QVector<double> yData);
    void updateChartSize(int value);
    void updatePointDensity(int density);
    void selectGraphColor();
    void logMessage(const QString &message); // Метод для логирования сообщений
    void saveChart(QChartView *chartView, const QString &fileName);
    void saveGraphsToFile();
    void setupAxes(QAbstractSeries *series, const QVector<double> &xData, const QVector<double> &yData, const QString &xTitle, const QString &yTitle);

    QLabel *createLabel(const QString &text);
    QSpinBox *createSizeSpinBox();
    QPushButton *createColorButton();

    Logger *m_logger;
    DatabaseManager *dbManager;

    int density;

    QList<GNRMCData> gnrmc;

    QChart *chart;
    QChartView *chartView;

    QWidget *chartsTab;
    QGridLayout *chartsLayout;
    QComboBox *graphSelector;
    QComboBox *flightComboBox;
    QCheckBox *borderCheckBox;
    QSpinBox *densitySpinBox;
    QComboBox *graphTypeSelector;

    QPushButton *saveButton;

    QCustomPlot *ChartPlot;
    QLineSeries *ChartSeries;

    QLineEdit *filterLineEdit; // Поле для ввода фильтра
    QComboBox *filterComboBox; // Выпадающий список для выбора поля фильтрации
    QComboBox *sortComboBox; // Выпадающий список для выбора поля сортировки
    QComboBox *orderComboBox; // Выпадающий список для выбора порядка сортировки
    QPushButton *filterButton; // Кнопка для применения фильтра

    QColor color; // Переменная для хранения выбранного цвета
};

#endif // SETUPCHARTSTAB_H

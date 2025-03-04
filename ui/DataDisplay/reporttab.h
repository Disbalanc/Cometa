#ifndef REPORTTAB_H
#define REPORTTAB_H

#include "aianalyzer.h"
#include "databasemanager.h"
#include "NavigationData.h"

#include <QWidget>
#include <QComboBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QTextEdit>
#include <QCheckBox>
#include <QGroupBox>
#include <QLineEdit>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QDateTimeAxis>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QBarSeries>
#include <QtCharts/QPieSeries>
#include <QTextDocument>
#include <QTextCursor>
#include <QBuffer>
#include <QFormLayout>
#include <QRadioButton>
#include <QFileDialog>
#include <QValueAxis>
#include <QButtonGroup>
#include <QSplineSeries>
#include <QSpinBox>
#include <QQuickView>
#include <QtLocation/QGeoServiceProvider>
#include <QtQuick/QQuickItem>
#include <QtDataVisualization/Q3DScatter>
#include <QtDataVisualization/QScatter3DSeries>
#include <QtQml/QQmlContext>
#include <QListWidget>
#include <QtMath>
#include <tuple>

QT_BEGIN_NAMESPACE
namespace QtCharts {
class QChart;
class QLineSeries;
}
namespace QtDataVisualization {
class Q3DScatter;
class QScatter3DSeries;
}
QT_END_NAMESPACE

class ReportTab : public QWidget {
    Q_OBJECT

public:
    explicit ReportTab(DatabaseManager *dbManager,Logger *logger, QWidget *parent = nullptr);

private slots:
    void onGenerateReport();
    void onAddBlock();
    void onSaveReport();
    void onAIAnalysisComplete(const QString &result);
    void onAIError(const QString &message);

private:
    // Основные компоненты
    DatabaseManager *dbManager;
    AIAnalyzer *aiAnalyzer;
    Logger *m_logger;

    // UI элементы
    QGroupBox *tableSettingsGroup;
    QGroupBox *textSettingsGroup;
    QGroupBox *mapSettingsGroup;
    QTextEdit *textEdit;
    QComboBox *yAxisSelector;
    QPushButton *addBlockButton;
    QPushButton *generateButton;
    QPushButton *saveButton;
    QTextEdit *reportTextEdit;
    QComboBox *flightComboBox;
    QListWidget *yAxisList;
    QSpinBox *densitySpinBox;
    QPushButton *colorButton;
    QGroupBox *chartSettingsGroup;
    QComboBox *chartTypeSelector;
    QComboBox *xAxisSelector;

    // Фильтрация и сортировка
    QComboBox *filterComboBox;
    QLineEdit *filterLineEdit;
    QComboBox *sortComboBox;
    QComboBox *orderComboBox;

    // Типы блоков
    QRadioButton *tableBlockRadio;
    QRadioButton *chartBlockRadio;
    QRadioButton *mapBlockRadio;
    QRadioButton *graphBlockRadio;
    QRadioButton *textBlockRadio;
    QRadioButton *aiTextBlockRadio;

    // Данные и состояние
    QString fligth_name;
    int figureCounter = 0;
    QColor chartColor;
    int pointDensity = 1;

    // Настройки карты
    QColor mapLineColor;
    int mapDensity;
    bool showMarkers;
    QString mapProvider;
    QString mapType;
    QQuickView *mapView;

    // 3D графика
    QtDataVisualization::Q3DScatter *scatter3D;
    QtDataVisualization::QScatter3DSeries *scatter3DSeries;
    QColor scatterColor;
    int scatterPointSize;
    int scatterDensity = 1;
    QGroupBox *scatter3DSettingsGroup;
    QComboBox *cameraProjectionCombo;

    // Основные методы
    void setupUI();
    void generateReport();
    void generateReportHeader();
    QList<NavigationData> getFilteredData();
    QString getLogoBase64();
    QPixmap renderMapToPixmap();

    // Генерация блоков
    void generateAITextBlock(QTextCursor &cursor);
    void generateTableBlock(QTextCursor &cursor);
    void generateChartBlock(QTextCursor &cursor);
    void generateTextBlock(QTextCursor &cursor);
    void generateMapBlock(QTextCursor &cursor);
    void generate3DChartBlock(QTextCursor &cursor);

    // Вспомогательные методы
    void saveAsHtml(const QString &fileName);
    void saveAsPdf(const QString &fileName);
    QString processHtmlForPdf(const QString &html);
    void saveAsDocx(const QString &fileName);
    void generatePieChart(QTextCursor &cursor);
    QString formatDuration(int seconds);
    void selectChartColor();
    void setupChart(QtCharts::QChart *chart, const QList<NavigationData> &data);
    QPixmap renderChartToPixmap(QtCharts::QChart *chart);
    void insertPixmapToDocument(const QPixmap &pixmap, QTextCursor &cursor);
    void setup3DChart(QtDataVisualization::Q3DScatter *chart, const QList<NavigationData> &data);
    QPixmap render3DChartToPixmap(QtDataVisualization::Q3DScatter *chart);
    // Новые методы для аналитики
    QString generateLegendHtml() const;
    std::tuple<double, double, double, double, double, int, double, double, double, double> calculateFlightMetrics(const QList<NavigationData>& data);
    double calculateDistance(const QList<NavigationData>& data);
};

#endif // REPORTTAB_H

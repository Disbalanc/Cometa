#ifndef DATADISPLAYWINDOW_H
#define DATADISPLAYWINDOW_H

#include "setupchartstab.h"
#include "setuptabletab.h"
#include "setupgraphtab.h"
#include "mapwidget.h"
#include "reporttab.h"

#include <QDialog>
#include <QVBoxLayout>
#include <QTableWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QHeaderView>
#include <QFile>
#include <QTextStream>
#include <QDateTime>

#include <Qt3DExtras/Qt3DWindow>
#include <QtWebEngineWidgets/QWebEngineView>
#include <Qt3DCore/QEntity>
#include <Qt3DCore/QTransform>
#include <Qt3DRender/QCamera>
#include <Qt3DRender/QPointLight>
#include <Qt3DExtras/QCylinderMesh>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DExtras/QOrbitCameraController>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QXmlStreamWriter>
#include <QInputDialog>
#include <Q3DScatter>
#include <QQuickView>

#include <QLabel>
#include <QSpinBox>
#include <QCheckBox>
#include <QColorDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QScatterSeries>
#include <QtDataVisualization/Q3DBars>
#include <QtDataVisualization/QValue3DAxis>
#include <QtDataVisualization/Q3DTheme>
#include <QtDataVisualization/QScatter3DSeries>
#include <QBarSeries>
#include <QQmlContext>

class DataDisplayWindow : public QDialog {
    Q_OBJECT

public:
    explicit DataDisplayWindow(DatabaseManager *db,Logger *logger,QWidget *parent = nullptr);
    void updateTheme(const QString &theme); // Метод для обновления темы

private:
    void logMessage(const QString &message); // Метод для логирования сообщений
    void setupParameterPanel(QVBoxLayout *mainLayout);


    QPushButton *closeButton; // Кнопка для закрытия окная
    DatabaseManager *dbManager; // Менеджер базы данных
    Logger* m_logger;

    setupTableTab *tableTab;
    setupChartsTab *chartsTab;
    setupGraphTab *graphTab;
    ReportTab *reportTab;
    MapWidget *mapWidget;
};

#endif // DATADISPLAYWINDOW_H

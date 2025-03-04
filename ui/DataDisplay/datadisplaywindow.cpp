#include "datadisplaywindow.h"
#include "mapwidget.h"

#include <QQmlApplicationEngine>
#include <QQuickItem>
#include <qchartview.h>

DataDisplayWindow::DataDisplayWindow(DatabaseManager *db,Logger *logger,QWidget *parent) : QDialog(parent),dbManager(db),m_logger(logger) {
    setWindowTitle("Данные из базы данных");
    setGeometry(100, 100, 800, 600);

    auto *mainLayout = new QVBoxLayout(this);
    auto *tabWidget = new QTabWidget(this);
    mainLayout->addWidget(tabWidget);
    setupParameterPanel(mainLayout);

    tableTab = new setupTableTab(dbManager,m_logger,this);
    tabWidget->addTab(tableTab, "Таблица");
    m_logger->log(Logger::Info,"tabWidget Таблица создался");

    graphTab = new setupGraphTab(dbManager,m_logger,this);
    tabWidget->addTab(graphTab, "3D График");
    m_logger->log(Logger::Info,"tabWidget 3D График создался");

    chartsTab = new setupChartsTab(dbManager,m_logger,this);
    tabWidget->addTab(chartsTab, "Графики");
    m_logger->log(Logger::Info,"tabWidget Графики создался");

    mapWidget = new MapWidget(dbManager,m_logger,this);
    tabWidget->addTab(mapWidget, "Карта");
    m_logger->log(Logger::Info,"tabWidget Карта создался");

    reportTab = new ReportTab(dbManager,m_logger,this);
    tabWidget->addTab(reportTab, "Отчет");
    m_logger->log(Logger::Info,"tabWidget Отчет создался");

    setLayout(mainLayout);
}

void DataDisplayWindow::setupParameterPanel(QVBoxLayout *mainLayout) {
    auto *parameterPanel = new QWidget(this);
    auto *parameterLayout = new QVBoxLayout(parameterPanel);

    // Кнопка для выхода
    auto *exitButton = new QPushButton("Выход", this);
    connect(exitButton, &QPushButton::clicked, this, &QWidget::close);
    parameterLayout->addWidget(exitButton);

    mainLayout->addWidget(parameterPanel);
}

void DataDisplayWindow::logMessage(const QString &message) {
    // Логирование сообщений в файл или консоль
    m_logger->log(Logger::Error,message);
}

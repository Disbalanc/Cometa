#include "mapwidget.h"

MapWidget::MapWidget(DatabaseManager* db,Logger *Logger, QWidget* parent)
    : QWidget(parent),dbManager(db),m_logger(Logger)
{
    if(m_logger) {
        m_logger->log(Logger::Info, "Инициализация MapWidget...");
    }
    // Создаем QQuickView
    QQuickView *view = new QQuickView();

    if(m_logger){
        view->rootContext()->setContextProperty("logger", m_logger);
        m_logger->log(Logger::Info, "Logger registered in QML context");
    }else {
        m_logger->log(Logger::Warning, "Logger is null - QML context registration skipped");
        return;
    }

    // Регистрация DatabaseManager для использования в QML
    if(dbManager) {
        view->rootContext()->setContextProperty("dbManager", dbManager);
        m_logger->log(Logger::Info, "DatabaseManager registered in QML context");
    } else {
        m_logger->log(Logger::Warning, "DatabaseManager is null - QML context registration skipped");
        return;
    }

    // Загружаем QML
    view->setSource(QUrl("qrc:/ui/DataDisplay/Map/MapView.qml")); // Убедитесь, что путь правильный
    if(view->status() == QQuickView::Error) {
        m_logger->log(Logger::Error, "Failed to load QML file: " + view->errors().first().description());
        return;
    }
    m_logger->log(Logger::Info, "QML resources loaded successfully");

    view->setResizeMode(QQuickView::SizeRootObjectToView);

    // Создаем контейнер для QQuickView
    QWidget *container = QWidget::createWindowContainer(view);
    QVBoxLayout *layout = new QVBoxLayout(this);

    layout->addWidget(container);
    setLayout(layout);
}

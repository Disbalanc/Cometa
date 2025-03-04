#ifndef MAPWIDGET_H
#define MAPWIDGET_H

#include <QQuickItem>
#include <QQuickView>
#include <QQmlContext>
#include <QWidget>
#include <QVBoxLayout>
#include <databasemanager.h>

class MapWidget : public QWidget {
    Q_OBJECT

public:
    MapWidget(DatabaseManager *db,Logger *logger,QWidget *parent = nullptr);

private:
    QQuickView *view;
    QQuickItem *rootObject;
    DatabaseManager *dbManager;
    Logger *m_logger;
};

#endif // MAPWIDGET_H

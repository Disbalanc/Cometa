#ifndef SETUPGRAPHTAB_H
#define SETUPGRAPHTAB_H

#include <Q3DScatter>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QScatter3DSeries>
#include <QVBoxLayout>
#include <QWidget>
#include <QColorDialog>
#include <QSpinBox>
#include <QLineEdit>
#include <QInputDialog>

#include "databasemanager.h"
#include "NavigationData.h"

class setupGraphTab : public QWidget
{
    Q_OBJECT
public:
    setupGraphTab(DatabaseManager *db,Logger *logger,QWidget *parent = nullptr);

    void saveGraphsToFile();
    void updateScatterGraph(QList<NavigationData> navigationDataList);
    void resetFilters();

private slots:
    void applyFilter(); // Слот для применения фильтра

private:
    void setupUI();
    void logMessage(const QString &message); // Метод для логирования сообщений
    void saveData();
    void saveGraphAsImage();
    void saveModelAsOBJ();
    void updatePointDensity(int density);

    QLabel *createLabel(const QString &text);

    QtDataVisualization::Q3DScatter *scatterGraph; // 3D график
    QtDataVisualization::QScatter3DSeries *series; // Серия данных для графика

    QComboBox *flightComboBox; // Комбобокс для выбора полета
    QPushButton *loadFlightButton; // Кнопка для загрузки данных
    void loadFlights(); // Метод для загрузки полетов в комбобокс

    QPushButton *saveButton;

    QLineEdit *filterLineEdit; // Поле для ввода фильтра
    QComboBox *filterComboBox; // Выпадающий список для выбора поля фильтрации
    QComboBox *sortComboBox; // Выпадающий список для выбора поля сортировки
    QComboBox *orderComboBox; // Выпадающий список для выбора порядка сортировки
    QPushButton *filterButton; // Кнопка для применения фильтра
    QSpinBox *densitySpinBox;

    Logger *m_logger;
    DatabaseManager *dbManager; // Менеджер базы данных

    int density;

    QColor color; // Переменная для хранения выбранного цвета
};

#endif // SETUPGRAPHTAB_H

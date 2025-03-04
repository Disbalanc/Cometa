#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QComboBox>
#include <QPushButton>
#include <QTextEdit>
#include <QLineEdit>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QMessageBox>
#include <QFileDialog>
#include <QProgressDialog>
#include <QSerialPort>
#include <QSerialPortInfo>
#include "connectionmanager.h"
#include "datamanager.h"
#include "parsernmea.h"
#include "logger.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QString dbPath, QWidget *parent = nullptr);
    ~MainWindow();
    QTextEdit *dataDisplay;

public slots:
    void showError(const QString &errorMessage);
    void onDatabasePathChanged(const QString &dbPath);

private slots:
    void onConnectionStatusChanged(bool connected);
    void onConnectButtonClicked();
    void onViewDataButtonClicked();
    void onSelectFileButtonClicked();
    void openSettings();
    void appendLogMessage(const QString &message);
    void appendFormattedData(const QString &data);

private:
    Logger *m_logger;
    void setupLogging();
    void styleLogDisplay();
    void setupUI();
    void updateInputFields();

    ConnectionManager *connectionManager;
    DatabaseManager *dbManager;
    DataManager *dataManager;
    ParserNMEA *parser;

    QComboBox *connectionTypeComboBox;
    QComboBox *serialPortComboBox;
    QLineEdit *ipAddressLineEdit;
    QLineEdit *portLineEdit;
    QPushButton *connectButton;
    QPushButton *viewDataButton;
    QPushButton *selectLogButton;

};

#endif // MAINWINDOW_H

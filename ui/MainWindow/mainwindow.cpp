#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "settings.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QGroupBox>
#include <QFormLayout>
#include <datadisplaywindow.h>

MainWindow::MainWindow(QString dbPath,QWidget *parent)
    : QMainWindow(parent),
    m_logger(new Logger(QDir::currentPath() + "/logs/logfile.log")),
    dbManager(new DatabaseManager(dbPath)),
    dataManager(new DataManager(dbManager, this)),
    connectionManager(new ConnectionManager(this))
 {

    setupLogging();
    setupUI();
    styleLogDisplay();

    connect(connectionManager, &ConnectionManager::errorOccurred, this, &MainWindow::showError);
    connect(dataManager, &DataManager::errorOccurred, this, &MainWindow::showError);
    connect(connectionManager, &ConnectionManager::dataFormatted,
            this, &MainWindow::appendFormattedData);
    connect(connectionManager, &ConnectionManager::connectionStatusChanged,
            this, &MainWindow::onConnectionStatusChanged);
    m_logger->log(Logger::Info, "Программа запущена");
}

void MainWindow::appendFormattedData(const QString &data) {
    dataDisplay->append("<pre>" + data + "</pre>"); // Для сохранения форматирования
}

void MainWindow::setupLogging()
{
    connect(m_logger, &Logger::logMessage, this, &MainWindow::appendLogMessage);

    dataManager->setLogger(m_logger);
    dbManager->setLogger(m_logger);
    parser->setLogger(m_logger);
    connectionManager->setLogger(m_logger);
}

void MainWindow::appendLogMessage(const QString &message)
{
    QString color;
    if (message.contains("[ERROR]") || message.contains("[CRITICAL]")) {
        color = "#ff0000";
    } else if (message.contains("[WARNING]")) {
        color = "#ffa500";
    } else if (message.contains("[INFO]")) {
        color = "#0000ff";
    } else {
        color = "#000000";
    }

    dataDisplay->append(QString("<span style='color:%1;'>%2</span>")
                            .arg(color)
                            .arg(message.toHtmlEscaped()));

    // Автопрокрутка к новым сообщениям
    QTextCursor cursor = dataDisplay->textCursor();
    cursor.movePosition(QTextCursor::End);
    dataDisplay->setTextCursor(cursor);
}

void MainWindow::styleLogDisplay()
{
    dataDisplay->setStyleSheet(
        "QTextEdit {"
        "   background-color: #f8f8f8;"
        "   font-size: 12pt;"
        "   border: 1px solid #cccccc;"
        "   padding: 5px;"
        "   font-family: 'Courier New', monospace;"
        "}"
        );
}

void MainWindow::onConnectButtonClicked() {
    static bool isConnected = false;

    try {
        if (!isConnected) {
            QString connectionType = connectionTypeComboBox->currentText();
            if (connectionType == "TTL") {
                QString selectedPort = serialPortComboBox->currentText();
                connectionManager->connectToTTL(selectedPort);
            } else if (connectionType == "Ethernet") {
                QString ipAddress = ipAddressLineEdit->text();
                QString portString = portLineEdit->text();
                bool ok;
                quint16 port = portString.toUShort(&ok);
                if (ipAddress.isEmpty() || !ok) {
                    throw std::runtime_error("Пожалуйста, введите корректный IP-адрес и порт.");
                }
                connectionManager->connectToEthernet(ipAddress, port);
            } else {
                throw std::runtime_error("Пожалуйста, выберите действительный тип соединения.");
            }
            isConnected = true;
            connectButton->setText("Disconnect");
        } else {
            connectionManager->disconnect();
            isConnected = false;
            connectButton->setText("Connect");
        }
        m_logger->log(Logger::Info, "Connection established successfully");
    } catch (const std::exception &e) {
        m_logger->log(Logger::Error,
                      QString("Connection failed: %1").arg(e.what()));
    }
}

MainWindow::~MainWindow() {
    connectionManager->disconnect();
    delete dataManager;
    delete m_logger;
}

void MainWindow::setupUI() {
    setWindowTitle("Cometa   ☄");
    setGeometry(100, 100, 800, 600);

    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);

    // Группа для выбора соединения
    QGroupBox *connectionGroup = new QGroupBox("Выбор соединения", this);
    QVBoxLayout *connectionLayout = new QVBoxLayout(connectionGroup);

    connectionTypeComboBox = new QComboBox(this);
    serialPortComboBox = new QComboBox(this);
    ipAddressLineEdit = new QLineEdit(this);
    portLineEdit = new QLineEdit(this);

    connectionTypeComboBox->addItems({"Select Connection Type", "TTL", "Ethernet"});
    connectionLayout->addWidget(connectionTypeComboBox);


    connectionLayout->addWidget(serialPortComboBox);
    connectionManager->populateSerialPorts(serialPortComboBox);

    // Ethernet fields
    QFormLayout *ethernetLayout = new QFormLayout();

    ipAddressLineEdit->setPlaceholderText("IP Address");
    ethernetLayout->addRow(ipAddressLineEdit);


    portLineEdit->setPlaceholderText("Port");
    ethernetLayout->addRow(portLineEdit);

    connectionLayout->addLayout(ethernetLayout);
    layout->addWidget(connectionGroup);

    // Группа для кнопок
    QGroupBox *buttonGroup = new QGroupBox("Действия", this);
    QVBoxLayout *buttonLayout = new QVBoxLayout(buttonGroup);

    viewDataButton = new QPushButton("Просмотреть данные", this);
    buttonLayout->addWidget(viewDataButton);
    connect(viewDataButton, &QPushButton::clicked, this, &MainWindow::onViewDataButtonClicked);

    connectButton = new QPushButton("Подключить", this);
    buttonLayout->addWidget(connectButton);
    connect(connectButton, &QPushButton::clicked, this, &MainWindow::onConnectButtonClicked);

    selectLogButton = new QPushButton("Выбрать лог-файл", this);
    buttonLayout->addWidget(selectLogButton);
    connect(selectLogButton, &QPushButton::clicked, this, &MainWindow::onSelectFileButtonClicked);

    // Кнопка для открытия окна настроек
    QPushButton *settingsButton = new QPushButton("Настройки", this);
    buttonLayout->addWidget(settingsButton);
    connect(settingsButton, &QPushButton::clicked, this, &MainWindow::openSettings);

    layout->addWidget(buttonGroup);

    // Data display area
    dataDisplay = new QTextEdit(this);
    dataDisplay->setReadOnly(true);
    layout->addWidget(dataDisplay);

    setCentralWidget(centralWidget);

    portLineEdit->setVisible(false);
    ipAddressLineEdit->setVisible(false);
    serialPortComboBox->setVisible(false);

    // Connect signals to slots
    connect(connectionTypeComboBox, &QComboBox::currentTextChanged, this, &MainWindow::updateInputFields);
}

void MainWindow::openSettings() {
    Settings *settingsWin = new Settings(this);
    settingsWin->exec(); // Open the window as modal
}

void MainWindow::onDatabasePathChanged(const QString &dbPath) {
    // Закрываем текущую базу данных, если она открыта
    dbManager->close();

    // Создаем новый экземпляр DatabaseManager с новым путем
    dbManager = new DatabaseManager(dbPath, this);
    dbManager->setLogger(m_logger);

    // Инициализируем базу данных
    dbManager->initializeDatabase();
}

void MainWindow::updateInputFields() {
    try{
        if (!serialPortComboBox || !ipAddressLineEdit || !portLineEdit) {
            showError("UI elements not initialized!");
            return;
        }

        QString connectionType = connectionTypeComboBox->currentText();
        if (connectionType == "TTL") {
            serialPortComboBox->setVisible(true);
            ipAddressLineEdit->setVisible(false);
            portLineEdit->setVisible(false);
            connectionManager->populateSerialPorts(serialPortComboBox);
        } else if (connectionType == "Ethernet") {
            serialPortComboBox->setVisible(false);
            ipAddressLineEdit->setVisible(true);
            portLineEdit->setVisible(true);
        } else {
            serialPortComboBox->setVisible(false);
            ipAddressLineEdit->setVisible(false);
            portLineEdit->setVisible(false);
        }
    }catch(const std::exception &e){
        showError(QString("Exception: %1").arg(e.what()));
    }

}

void MainWindow::showError(const QString &errorMessage) {
    m_logger->log(Logger::Error, errorMessage); // Логируем ошибку
}

void MainWindow::onViewDataButtonClicked() {
    DataDisplayWindow *dataWindow = new DataDisplayWindow(dbManager,m_logger, this);
    dataWindow->exec(); // Open the window as modal
}

void MainWindow::onSelectFileButtonClicked() {
    QString filePath = QFileDialog::getOpenFileName(this, "Выберите файл", "", "Log Files (*.log);;Batch Files (*.bat);;Text Files (*.txt);;All Files (*)");
    if (!filePath.isEmpty()) {
        dataManager->processLogFile(filePath);
    }
}

void MainWindow::onConnectionStatusChanged(bool connected) {
    if (connected){
        dbManager->insertNewFlight();
        dataManager->saveFile(dbManager->getLastFlight());
    }
}

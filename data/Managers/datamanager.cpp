#include "datamanager.h"

#include "mainwindow.h"

#include <QMessageBox>
#include <QProgressDialog>

DataManager::DataManager(DatabaseManager *dbManager, QObject *parent)
    : QObject(parent), dbManager(dbManager) {

}

// В реализации DataManager.cpp:
void DataManager::processLogFile(const QString& filePath) {
    if (isProcessing) return;
    isProcessing = true;

    parser.setLogger(m_logger);

    try {
        currentFile = new QFile(filePath);
        if (!currentFile->open(QIODevice::ReadOnly | QIODevice::Text)) {
            emit errorOccurred("Не удалось открыть файл.");
            delete currentFile;
            currentFile = nullptr;
            isProcessing = false;
            return;
        }

        startTime = std::chrono::high_resolution_clock::now();
        dbManager->insertNewFlight();

        processedCount = 0;
        savedCount = 0;
        minLatitude = std::numeric_limits<double>::max();
        maxLatitude = std::numeric_limits<double>::lowest();
        minLongtitude = std::numeric_limits<double>::max();
        maxLongtitude = std::numeric_limits<double>::lowest();
        minAltitude = std::numeric_limits<double>::max();
        maxAltitude = std::numeric_limits<double>::lowest();

        QTimer::singleShot(0, this, &DataManager::processNextBlock);
    } catch (const std::exception& e) {
        emit errorOccurred("Ошибка обработки файла: " + QString::fromStdString(e.what()));
        isProcessing = false;
    }
}

void DataManager::processNextBlock() {
    if (!currentFile || !currentFile->isOpen()) {
        finishProcessing();
        return;
    }

    const int blockSize = 8192;
    QByteArray buffer = currentFile->read(blockSize);
    if (buffer.isEmpty()) {
        finishProcessing();
        return;
    }

    QList<QString> lines = QString(buffer).split('\n', QString::SkipEmptyParts);
    for (QString& line : lines) {
        NavigationData navData = parser.parseData(line);
        m_logger->log(Logger::Info, "Processed:\n" + line);
        processedCount++;
        if (navData.result == OK) {
            saveNavigationData(navData);
            savedCount++;

            if (navData.type == GNGGA) {
                GNGGAData gngga;
                QDataStream stream(&navData.data, QIODevice::ReadOnly);
                stream >> gngga.latitude >> gngga.longitude
                    >> gngga.satellitesCount >> gngga.HDOP >> gngga.altitude;

                minLatitude = std::min(minLatitude, static_cast<double>(gngga.latitude));
                maxLatitude = std::max(maxLatitude, static_cast<double>(gngga.latitude));
                minLongtitude = std::min(minLongtitude, static_cast<double>(gngga.longitude));
                maxLongtitude = std::max(maxLongtitude, static_cast<double>(gngga.longitude));
                minAltitude = std::min(minAltitude, static_cast<double>(gngga.altitude));
                maxAltitude = std::max(maxAltitude, static_cast<double>(gngga.altitude));
            }
        }
    }

    QTimer::singleShot(0, this, &DataManager::processNextBlock);
}

void DataManager::finishProcessing() {
    currentFile->close();
    delete currentFile;
    currentFile = nullptr;

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - startTime;

    QString summary = QString("Обработка завершена.\n")
                      + QString("Самая низкая высота: %1 метров\n").arg(minAltitude)
                      + QString("Самая высокая высота: %1\n").arg(maxAltitude)
                      + QString("Самая низкая широта: %1\n").arg(minLatitude)
                      + QString("Самая высокая широта: %1\n").arg(maxLatitude)
                      + QString("Самая низкая долгота: %1\n").arg(minLongtitude)
                      + QString("Самая высокая долгота: %1\n").arg(maxLongtitude)
                      + QString("Обработано пакетов: %1\n").arg(savedCount)
                      + QString("Обработка файла заняла %1 секунд.").arg(duration.count());

    QMetaObject::invokeMethod(this, "showSummaryMessage",
                              Qt::QueuedConnection, Q_ARG(QString, summary));

    emit dataProcessed(processedCount, savedCount, duration);
    isProcessing = false;
}

void DataManager::showSummaryMessage(const QString& summary) {
    QMessageBox msgBox;
    msgBox.setText(summary);
    msgBox.setWindowTitle("Файл обработан");
    msgBox.exec();
}

void DataManager::saveNavigationData(const NavigationData &navData) {
    try {
        if (!dbManager->saveNavigationData(navData)) {
            emit errorOccurred("Не удалось сохранить навигационные данные для типа: " + QString::number(navData.type));
        }
    } catch (const std::exception &e) {
        emit errorOccurred("Ошибка сохранения данных для типа: " + QString::number(navData.type) + " - " + QString::fromStdString(e.what()));
    }
}

void DataManager::saveFile(const QString &flightName) {
    QString dirPath = QDir::currentPath() + "/data"; // Путь к директории
    QDir().mkpath(dirPath); // Создаем директорию, если она не существует

    filePath = dirPath + "/" + flightName + ".bin"; // Путь к файлу с именем полета
}

void DataManager::setLogger(Logger *logger) {
    m_logger = logger;
}

void DataManager::writeDataToFile(const QByteArray &data) {
    QFile file(filePath);

    if (!file.open(QIODevice::Append | QIODevice::WriteOnly)) {
        m_logger->log(Logger::Error, // Добавляем уровень ошибки
                    "Не удалось открыть файл для записи: " + file.errorString());
        return;
    }

    qint64 bytesWritten = file.write(data);
    if (bytesWritten == -1) {
        m_logger->log(Logger::Error, // Добавляем уровень ошибки
                    "Ошибка записи в файл: " + file.errorString());
    } else {
        m_logger->log(Logger::Debug, // Добавляем уровень отладки
                    QString("Записано %1 байт в файл").arg(bytesWritten));
    }

    file.close();
}

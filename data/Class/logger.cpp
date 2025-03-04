// logger.cpp
#include "logger.h"
#include <QDateTime>
#include <QTextStream>
#include <QDir>
#include <QDebug>
#include <QTextEdit>

Logger::Logger(const QString &filePath, QObject *parent)
    : QObject(parent)
{
    QDir().mkpath(QDir::currentPath() + "/logs");
    m_logFile.setFileName(filePath);
    if (!m_logFile.open(QIODevice::Append | QIODevice::Text)) {
        qCritical("Cannot open log file: %s", qUtf8Printable(filePath)); // Исправленный формат
    }
}

Logger::~Logger()
{
    if (m_logFile.isOpen()) {
        m_logFile.close();
    }
}

void Logger::log(Logger::LogLevel level, const QString &message)
{
    //QMutexLocker locker(&m_mutex);
    const QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    const QString levelStr = logLevelToString(level);
    const QString formatted = QString("[%1] [%2] %3")
                                  .arg(timestamp)
                                  .arg(levelStr)
                                  .arg(message);

    // Запись в файл
    writeToFile(formatted);

    // Отправка в GUI
    emit logMessage(formatted);

    // Дублирование в консоль для отладки
    qDebug().noquote() << formatted; // Явное указание формата вывода
}

void Logger::setDisplayWidget(QTextEdit *widget)
{
    m_displayWidget = widget;
}

QString Logger::logLevelToString(LogLevel level)
{
    switch(level) {
    case Debug:    return "DEBUG";
    case Info:     return "INFO";
    case Warning:  return "WARNING";
    case Error:    return "ERROR";
    case Critical: return "CRITICAL";
    default:       return "UNKNOWN";
    }
}

void Logger::writeToFile(const QString &message)
{
    if (m_logFile.isOpen()) {
        QTextStream stream(&m_logFile);
        stream << message << "\n";
        stream.flush();
    }
}

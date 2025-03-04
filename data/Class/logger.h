// logger.h
#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QString>
#include <QFile>
#include <QMutex>
#include <QTextEdit>

class Logger : public QObject
{
    Q_OBJECT
    Q_ENUMS(LogLevel)
public:
    enum LogLevel {
        Debug,
        Info,
        Warning,
        Error,
        Critical
    };
    Q_ENUM(LogLevel)

    explicit Logger(const QString &filePath, QObject *parent = nullptr);
    ~Logger();

    Q_INVOKABLE void log(LogLevel level, const QString &message);
    void setDisplayWidget(QTextEdit *widget);

signals:
    void logMessage(const QString &formattedMessage);

private:
    QFile m_logFile;
    QTextEdit *m_displayWidget = nullptr;
    QMutex m_mutex;
    QString logLevelToString(LogLevel level);
    void writeToFile(const QString &message);
};

#endif // LOGGER_H

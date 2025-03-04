#pragma once // Добавить в самое начало файла

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class AIAnalyzer : public QObject {
    Q_OBJECT
public:
    explicit AIAnalyzer(QObject *parent = nullptr);
    void analyzeData(const QString &jsonData);
    void analyzeDataBlack(const QString &jsonData);

signals:
    void analysisComplete(const QString &result);
    void errorOccurred(const QString &message);

private:
    QNetworkAccessManager *manager;
    const QString API_URL = "https://api.example.com"; // Ваш URL API
    const QString API_TOKEN = "YOUR_API_TOKEN";
};

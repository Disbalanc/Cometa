#include "aianalyzer.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray> // Добавляем недостающий заголовок
#include <QSslSocket>

AIAnalyzer::AIAnalyzer(QObject *parent) : QObject(parent) {
    manager = new QNetworkAccessManager(this);
}

void AIAnalyzer::analyzeDataBlack(const QString &jsonData) {
    // Проверка поддержки SSL
    if (!QSslSocket::supportsSsl()) {
        qDebug() << "SSL not supported!" << QSslSocket::sslLibraryBuildVersionString();
    }
    // Исправляем создание QNetworkRequest
    QNetworkRequest request;
    request.setUrl(QUrl("https://api.blackbox.ai/api/v1/analyze"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject body;
    body["data"] = QJsonDocument::fromJson(jsonData.toUtf8()).array();

    QNetworkReply *reply = manager->post(request, QJsonDocument(body).toJson());

    connect(reply, &QNetworkReply::finished, [this, reply]() {
        if(reply->error() == QNetworkReply::NoError) {
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            emit analysisComplete(doc["result"].toString());
        } else {
            emit errorOccurred(reply->errorString());
        }
        reply->deleteLater();
    });
}

void AIAnalyzer::analyzeData(const QString &jsonData) {
    // Проверка поддержки SSL
    if (!QSslSocket::supportsSsl()) {
        qDebug() << "SSL not supported!" << QSslSocket::sslLibraryBuildVersionString();
    }
    // Формируем промпт
    QString prompt = QString(
                         "Анализируй данные полета и сгенерируй отчет на русском. "
                         "Данные в JSON:\n%1\n\n"
                         "Отчет должен содержать: общую характеристику, основные параметры, аномалии и выводы."
                         ).arg(jsonData);

    // Исправляем создание QNetworkRequest
    QNetworkRequest request;
    request.setUrl(QUrl(API_URL));
    request.setRawHeader("Authorization", QString("Bearer %1").arg(API_TOKEN).toUtf8());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject body;
    body["inputs"] = prompt;
    body["parameters"] = QJsonObject{
        {"max_new_tokens", 1000},
        {"temperature", 0.7}
    };

    QNetworkReply *reply = manager->post(request, QJsonDocument(body).toJson());

    connect(reply, &QNetworkReply::finished, [this, reply]() {
        if(reply->error() != QNetworkReply::NoError) {
            emit errorOccurred(reply->errorString());
            reply->deleteLater();
            return;
        }

        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        QString result = doc["generated_text"].toString();
        emit analysisComplete(result);
        reply->deleteLater();
    });
}

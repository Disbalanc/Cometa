// parsernmea.h
#ifndef PARSERNMEA_H
#define PARSERNMEA_H

#include "logger.h"
#include "NavigationData.h"
#include <QObject>
#include <QHash>
#include <QVector>
#include <QStringRef>
#include <QByteArray>

class ParserNMEA : public QObject
{
    Q_OBJECT
public:
    explicit ParserNMEA(QObject *parent = nullptr);
    void setLogger(Logger *logger);
    NavigationData parseData(QString &line);

private:
    // Методы серилизации
    QByteArray serialize(const GNRMCData &data);
    QByteArray serialize(const GNGGAData &data);
    QByteArray serialize(const GNGSAData &data);
    QByteArray serialize(const GNZDAData &data);
    QByteArray serialize(const GNDHVData &data);
    QByteArray serialize(const GNGSTData &data);
    QByteArray serialize(const GPTXTData &data);
    QByteArray serialize(const GNGLLData &data);
    QByteArray serialize(const GLGSVData &data);
    QByteArray serialize(const GNVTGData &data);

    // Парсеры для каждого типа сообщений
    void parseGNRMC(const QVector<QStringRef> &parts, NavigationData &data);
    void parseGNGGA(const QVector<QStringRef> &parts, NavigationData &data);
    void parseGNGSA(const QVector<QStringRef> &parts, NavigationData &data);
    void parseGNZDA(const QVector<QStringRef> &parts, NavigationData &data);
    void parseGNDHV(const QVector<QStringRef> &parts, NavigationData &data);
    void parseGNGST(const QVector<QStringRef> &parts, NavigationData &data);
    void parseGPTXT(const QVector<QStringRef> &parts, NavigationData &data);
    void parseGNGLL(const QVector<QStringRef> &parts, NavigationData &data);
    void parseGLGSV(const QVector<QStringRef> &parts, NavigationData &data);
    void parseGNVTG(const QVector<QStringRef> &parts, NavigationData &data);

    // Вспомогательные методы
    bool isValidString(QString &line);
    bool validateChecksum(const QString &line) const;
    void handleBuffer(QString &line);
    QTime parseTime(const QStringRef &ref,
                    const QString &msgType) const;
    QTime parseTimeWithMilliseconds(const QStringRef& timeRef,
                                    const QString& msgType) const;
    QDate parseDate(const QStringRef& dayRef,
                    const QStringRef& monthRef,
                    const QStringRef& yearRef,
                    const QString& msgType) const;
    QDate parseDate(const QStringRef &ref,
                    const QString& msgType) const;
    double parseDouble(const QStringRef &ref,
                       const QString &fieldName,
                       const QString &msgType) const;
    int parseInt(const QStringRef &ref,
                 const QString &fieldName,
                 const QString &msgType) const;
    QString parseString(const QStringRef &ref,
                        const QString &fieldName,
                        const QString &msgType) const;
    double parseCoordinate(const QStringRef &coord,
                           const QStringRef &dir,
                           const QString &msgType) const;

    // Валидация
    double validateAngle(double degrees,
                         const QString& field,
                         const QString& context) const;
    double validateRange(double value,
                         double min,
                         double max,
                         const QString &fieldName,
                         const QString &msgType) const;
    int validateRange(int value,
                      int min,
                      int max,
                      const QString &fieldName,
                      const QString &msgType) const;
    double validateNonNegative(double value,
                               const QString& field,
                               const QString& context) const;


    // Логирование
    void logError(const QString &msgType, const QString &message) const;
    void logWarning(const QString &msgType, const QString &message) const;
    void logInfo(const QString &msgType, const QString &message) const;

    QString inCompleteLine ="";

    Logger *m_logger = nullptr;
    QString m_buffer;
    QHash<QString, std::function<void(ParserNMEA*, const QVector<QStringRef>&, NavigationData&)>> m_parsers;
};

#endif // PARSERNMEA_H

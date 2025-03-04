#ifndef NAVIGATIONDATAFORMATTER_H
#define NAVIGATIONDATAFORMATTER_H

#include "parsernmea.h"
#include <QDataStream>
#include <QList>
#include <QPair>
#include <QString>
#include <QJsonArray>
#include <QJsonDocument>
#include <QStringBuilder>

class NavigationDataFormatter : public QObject {
    Q_OBJECT
public:
    explicit NavigationDataFormatter(QObject *parent = nullptr);
    QString formatNavigationData(const NavigationData &navData);

private:
    QString formatNavDataByType(const NavigationData &navData);
    QString formatGNRMCData(const GNRMCData &data);
    QString formatGNGGAData(const GNGGAData &data);
    QString formatGSAData(const GNGSAData &data);
    QString formatGNZDAData(const GNZDAData &data);
    QString formatGNDHVData(const GNDHVData &data);
    QString formatGNGSTData(const GNGSTData &data);
    QString formatGPTXTData(const GPTXTData &data);
    QString formatGNGLLData(const GNGLLData &data);
    QString formatGLGSVData(const GLGSVData &data);
    QString formatGNVTGData(const GNVTGData &data);

    QString formatCoordinate(double value, bool isLatitude);
    QString gnssTypeToString(TypeGNSS type);

    QHash<MsgType, QString> m_typeNames; // Кэширование имен типов сообщений
};

#endif // NAVIGATIONDATAFORMATTER_H

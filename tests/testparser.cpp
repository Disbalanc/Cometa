#include "testparser.h"

//// Тест для данных в двух строках
//TEST_F(ParserNMEATest, DataInTwoLines) {
//    QString line1 = "$GNRMC,052714.00,A,5624.91149,";
//    QString line2 = "N,06153.42199,E,0.120,,061224,,,A,V*16";
//    parser.parseData(line1,BLOCK); // Парсим первую строку
//    NavigationData navData = parser.parseData(line2,BLOCK); // Парсим вторую строку
//    EXPECT_TRUE(navData.gnrmcData.result == OK); // Проверяем, что данные валидны
//}

//// Тест для смешанного порядка данных
//TEST_F(ParserNMEATest, MixedDataOrder) {
//    QByteArray str = "$GNRMC,052714.00,A,5624.91149,N,06153.42199,E,0.120,,061224,,,A,V*16\n"
//                    "$GNGGA,052714.00,5624.91149,N,06153.42199,E,1,06,1.27,204.2,M,-12.4,M,,*6C\n"
//                    "$GNGSA,M,3,,,,,,,,,,,,,2.33,1.27,1.95,1*06\n"
//                    "$GNGSA,M,3,67,77,79,69,68,78,,,,,,,2.33,1.27,1.95,2*04\n"
//                    "$GNGSA,M,3,,,,,,,,,,,,,2.33,1.27,1.95,3*04\n"
//                    "$GNGSA,M,3,,,,,,,,,,,,,2.33,1.27,1.95,4*03\n"
//                    "$GNZDA,052714.00,06,12,2024,00,00*7C";
//   NavigationData navData;
//    QStringList lines = QString::fromUtf8(str).split('\n', QString::SkipEmptyParts);
//    for (QString &line : lines) {
//        // Парсим данные
//        navData = parser.parseData(line,BLOCK);
//    }
//    EXPECT_TRUE(navData.result); // Проверяем, что результат OK и все данные отпарсились
//}

//// Тест для валидных данных GNRMC
//TEST_F(ParserNMEATest, ValidGNRMCData) {

//    QString line = "$GNRMC,052712.00,A,5624.91014,N,06153.41794,E,0.302,,061224,,,A,V*13";
//    NavigationData navData = parser.parseData(line,BLOCK);
//    EXPECT_TRUE(navData.gnrmcData.isValid); // Проверяем, что данные валидны
//    EXPECT_EQ(navData.gnrmcData.latitude, 56.4152); // Проверяем широту
//    EXPECT_EQ(navData.gnrmcData.longitude,61.8903); // Проверяем долготу
//}

//// Тест для невалидных данных GNRMC
//TEST_F(ParserNMEATest, InvalidGNRMCData) {
//    QString line = "$GNRMC,052712.00,A,А,N,F4,E,0.302,,061224,,,A,V*13";
//    NavigationData navData = parser.parseData(line,BLOCK);
//    EXPECT_EQ(navData.result, ERROR); // Проверяем, что результат ERROR
//}

//// Тест для неполных данных
//TEST_F(ParserNMEATest, IncompleteData) {
//    QString line = "$GNRMC,123456.00,A,1234.56,N,12345.67,E,0.0,0.0"; // Без контрольной суммы
//    NavigationData navData = parser.parseData(line,BLOCK);
//    EXPECT_EQ(navData.result, ERROR); // Проверяем, что результат ERROR
//}



//// Тест для неверной контрольной суммы
//TEST_F(ParserNMEATest, InvalidChecksum) {
//    QString line = "$GNRMC,123456.00,A,1234.56,N,12345.67,E, 0.0,0.0,010101*00"; // Неверная контрольная сумма
//    NavigationData navData = parser.parseData(line,STRING);
//    EXPECT_EQ(navData.result, ERROR); // Проверяем, что результат ERROR
//}

//// Тест для данных с некорректной широтой
//TEST_F(ParserNMEATest, InvalidLatitude) {
//    QString line = "$GNRMC,123456.00,A,9999.99,N,12345.67,E,0.0,0.0,010101*7A"; // Некорректная широта
//    NavigationData navData = parser.parseData(line,STRING);
//    EXPECT_EQ(navData.result, ERROR); // Проверяем, что результат ERROR
//}

//// Тест для данных с некорректной долготой
//TEST_F(ParserNMEATest, InvalidLongitude) {
//    QString line = "$GNRMC,123456.00,A,1234.56,N,99999.99,E,0.0,0.0,010101*7A"; // Некорректная долгота
//    NavigationData navData = parser.parseData(line,STRING);
//    EXPECT_EQ(navData.result, ERROR); // Проверяем, что результат ERROR
//}

//// Тест для данных с некорректной скоростью
//TEST_F(ParserNMEATest, InvalidSpeed) {
//    QString line = "$GNRMC,123456.00,A,1234.56,N,12345.67,E,-10.0,0.0,010101*7A"; // Негативная скорость
//    NavigationData navData = parser.parseData(line,STRING);
//    EXPECT_EQ(navData.result, ERROR); // Проверяем, что результат ERROR
//}

//// Тест для данных с некорректным курсом
//TEST_F(ParserNMEATest, InvalidCourse) {
//    QString line = "$GNRMC,123456.00,A,1234.56,N,12345.67,E,0.0,-10.0,010101*7A"; // Негативный курс
//    NavigationData navData = parser.parseData(line,STRING);
//    EXPECT_EQ(navData.result, ERROR); // Проверяем, что результат ERROR
//}

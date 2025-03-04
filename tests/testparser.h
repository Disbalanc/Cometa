#ifndef TESTPARSER_H
#define TESTPARSER_H

#include <gtest/gtest.h>
#include "parsernmea.h" // Подключаем ваш парсер

class ParserNMEATest : public ::testing::Test {
protected:
    ParserNMEA parser; // Экземпляр парсера для тестов
};

#endif // TESTPARSER_H

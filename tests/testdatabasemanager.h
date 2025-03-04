#ifndef TESTDATABASMANAGER_H
#define TESTDATABASMANAGER_H

#include <gtest/gtest.h>
#include <QDir>
#include "databasemanager.h"

class TestDataBaseManager : public ::testing::Test {
protected:
    DatabaseManager *dbManager;

};

#endif // TESTDATABASMANAGER_H

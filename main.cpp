#include <QApplication>
#include <QThread>
#include <QDebug>
#include <QQuickView>
#include <QQmlApplicationEngine>
#include <gtest/gtest.h>
#include <QSettings>
#include <mainwindow.h>
#include <settings.h>

// Класс для запуска тестов в отдельном потоке
class TestRunner : public QThread {
protected:
    void run() override {
        ::testing::InitGoogleTest();
        int result = RUN_ALL_TESTS(); // Запуск всех тестов
        qDebug() << "Тесты завершены с кодом:" << result;
        QCoreApplication::exit(result); // Завершение приложения с кодом результата тестов
    }
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Регистрация DatabaseManager для использования в QML
    //qmlRegisterType<DatabaseManager>("mydatabase", 1, 0, "DatabaseManager");

    // Регистрация типа Logger для QML
    qmlRegisterUncreatableType<Logger>("App.Logging", 1, 0, "Logger",
                                       "Cannot create Logger instances in QML");
    qRegisterMetaType<Logger::LogLevel>("LogLevel");
    // Запуск тестов в отдельном потоке
    TestRunner testRunner;
    //testRunner.start();

    QSettings settings("Cometa", "Cometa");
    QString dbPath = settings.value("databasePath", QDir::currentPath() + "/database/mydatabase.db").toString();
    Settings::applyTheme(settings.value("theme","Стандартная тема").toString());
    Settings::applyLanguage(settings.value("language","Русский").toString());
    Settings::applyFontSize(settings.value("fontSize", 12).toInt());

    MainWindow *w = new MainWindow(dbPath);
    w->show();

    return a.exec();
}

#include "settings.h"
#include "ui_settings.h"
#include <QComboBox>
#include <QVBoxLayout>
#include <QPushButton>
#include <QSpinBox>
#include <QFileDialog>
#include <QLabel>
#include <QDebug>
#include <QSettings>

Settings::Settings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Settings),
    translator(new QTranslator(this)) // Создаем экземпляр QTranslator
{
    ui->setupUi(this);
    setWindowTitle("Настройки");

    // Инициализация dbPathLineEdit
    dbPathLineEdit = new QLineEdit(this);
    dbPathLineEdit->setReadOnly(true); // Делаем поле только для чтения

    QPushButton *selectDbButton = new QPushButton("Выбрать базу данных", this);

    // Остальная часть конструктора...
    // Создание комбобокса для выбора темы
    themeComboBox = new QComboBox(this);
    themeComboBox->addItems({"Стандартная тема", "Светлая тема", "Темная тема", "Ночная тема",
                             "Синяя тема", "Зеленая тема", "Розовая тема",
                             "Темно-синяя тема", "Монохромная тема",
                             "Пастельная тема", "Высококонтрастная тема"});
    connect(themeComboBox, &QComboBox::currentTextChanged, this, &Settings::onThemeChanged);

    // Создание комбобокса для выбора языка
    // languageComboBox = new QComboBox(this);
    // languageComboBox->addItems({"Русский", "English"});
    // connect(languageComboBox, &QComboBox::currentTextChanged, this, &Settings::onLanguageChanged);

    // Создание QSpinBox для выбора размера шрифта
    fontSizeSpinBox = new QSpinBox(this);
    fontSizeSpinBox->setRange(8, 30);
    fontSizeSpinBox->setValue(12); // Установить значение по умолчанию
    connect(fontSizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &Settings::onFontSizeChanged);

    // Кнопка для закрытия окна настроек
    QPushButton *closeButton = new QPushButton("Закрыть", this);
    connect(closeButton, &QPushButton::clicked, this, &Settings::accept);

    // Установка компоновки
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(themeComboBox);
    // layout->addWidget(languageComboBox);
    layout->addWidget(new QLabel("Размер шрифта:", this));
    layout->addWidget(fontSizeSpinBox);
    layout->addWidget(new QLabel("Путь к базе данных:", this));
    layout->addWidget(dbPathLineEdit);
    layout->addWidget(selectDbButton);
    layout->addWidget(closeButton);
    setLayout(layout);


    // Подключение сигналов
    connect(themeComboBox, &QComboBox::currentTextChanged, this, [this](const QString &text){
        onThemeChanged(text);
        saveSettingsTheme(); // Сохраняем при изменении
    });

    // connect(languageComboBox, &QComboBox::currentTextChanged, this, [this](const QString &text){
    //     onLanguageChanged(text);
    //     saveSettingsLanguage();
    // });

    connect(fontSizeSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int value){
        onFontSizeChanged(value);
        saveSettingsFontSize();
    });

    connect(selectDbButton, &QPushButton::clicked, this, [this]() {
        QString dbPath = QFileDialog::getOpenFileName(this, "Выберите базу данных", QDir::currentPath(), "SQLite Database (*.db)");
        if (!dbPath.isEmpty()) {
            dbPathLineEdit->setText(dbPath);
            saveSettingsDataBase();
            emit databasePathChanged(dbPath);
        }
    });

    // Загрузка настроек
    loadSettings();
}

Settings::~Settings() {
    delete ui;
}

void Settings::loadSettings() {
    QSettings settings("Cometa", "Cometa");

    qDebug() << "Загрузка настроек...";

    QString dbPath = settings.value("databasePath", " ").toString();
    qDebug() << "dbPath:" << dbPath;

    QString theme = settings.value("theme", "Стандартная тема").toString();
    qDebug() << "theme:" << theme;

    QString language = settings.value("language", "Русский").toString();
    qDebug() << "language:" << language;

    int fontSize = settings.value("fontSize", 12).toInt();
    qDebug() << "fontSize:" << fontSize;

    try{
    // Установка загруженных значений
    if (dbPathLineEdit) {
        dbPathLineEdit->setText(dbPath);
    } else {
        qDebug() << "dbPathLineEdit is nullptr!";
    }

    if (themeComboBox) {
        themeComboBox->setCurrentText(theme);
    } else {
        qDebug() << "themeComboBox is nullptr!";
    }

    // if (languageComboBox) {
    //     languageComboBox->setCurrentText(language);
    // } else {
    //     qDebug() << "languageComboBox is nullptr!";
    // }

    if (fontSizeSpinBox) {
        //fontSizeSpinBox->setValue(fontSize);
    } else {
        qDebug() << "fontSizeSpinBox is nullptr!";
    }}catch (const std::exception& e) {
        qDebug()<<"ошибка";
    }
}

void Settings::applyTheme(const QString& theme) {
    if (theme == "Стандартная тема") {
        Settings::setDefaultTheme();
    } else if (theme == "Светлая тема") {
        Settings::setLightTheme();
    } else if (theme == "Темная тема") {
        Settings::setDarkTheme();
    } else if (theme == "Ночная тема") {
        Settings::setNightTheme();
    } else if (theme == "Синяя тема") {
        Settings::setBlueTheme();
    } else if (theme == "Зеленая тема") {
        Settings::setGreenTheme();
    } else if (theme == "Розовая тема") {
        Settings::setPinkTheme();
    } else if (theme == "Темно-синяя тема") {
        Settings::setDarkBlueTheme();
    } else if (theme == "Монохромная тема") {
        Settings::setMonochromeTheme();
    } else if (theme == "Пастельная тема") {
        Settings::setPastelTheme();
    } else if (theme == "Высококонтрастная тема") {
        Settings::setHighContrastTheme();
    } else if (theme == "Аналоговая цветовая схема") {
        Settings::setAnalogousColorScheme();
    } else if (theme == "Цветовая схема триада") {
        Settings::setTriadicColorScheme();
    } else if (theme == "Дополнительная цветовая схема") {
        Settings::setComplementaryColorScheme();
    } else if (theme == "Сплит-комплементарной") {
        Settings::setSplitComplementaryColorScheme();
    } else if (theme == "Креативные цветовые схемы") {
        Settings::setCreativeColorSchemes();
    }
}

void Settings::applyLanguage(const QString& lang) {
    QTranslator translator;
    if (lang == "English") {
        translator.load(":/translations/Cometa_en_EN.ts");
    } else {
        translator.load(":/translations/Cometa_ru_RU.ts");
    }
    qApp->installTranslator(&translator);
}

void Settings::applyFontSize(int size) {
    QFont font = qApp->font();
    font.setPointSize(size);
    qApp->setFont(font);
}

void Settings::saveSettingsDataBase() {
    QSettings settings("Cometa", "Cometa");
    settings.setValue("databasePath", dbPathLineEdit->text());
}

void Settings::saveSettingsTheme() {
    QSettings settings("Cometa", "Cometa");
    settings.setValue("theme", themeComboBox->currentText());
}
void Settings::saveSettingsLanguage() {
    // QSettings settings("Cometa", "Cometa");
    // settings.setValue("language", languageComboBox->currentText());
}
void Settings::saveSettingsFontSize() {
    QSettings settings("Cometa", "Cometa");
    settings.setValue("fontSize", fontSizeSpinBox->value());
}

void Settings::onFontSizeChanged(int size) {
    QFont font = qApp->font(); // Получаем текущий шрифт приложения
    font.setPointSize(size); // Устанавливаем новый размер шрифта
    qApp->setFont(font); // Применяем новый шрифт ко всему приложению
}

void Settings::onLanguageChanged(const QString &language) {
    if (language == "English") {
        translator->load(":/translations/Cometa_en_EN.ts");
    } else {
        translator->load(":/translations/Cometa_ru_RU.ts");
    }
    qApp->installTranslator(translator); // Устанавливаем переводчик
    // Здесь можно добавить логику для обновления интерфейса после изменения языка
}

void Settings::onThemeChanged(const QString &theme) {
    if (theme == "Стандартная тема") {
        setDefaultTheme();
    } else if (theme == "Светлая тема") {
        setLightTheme();
    } else if (theme == "Темная тема") {
        setDarkTheme();
    } else if (theme == "Ночная тема") {
        setNightTheme();
    } else if (theme == "Синяя тема") {
        setBlueTheme();
    } else if (theme == "Зеленая тема") {
        setGreenTheme();
    } else if (theme == "Розовая тема") {
        setPinkTheme();
    } else if (theme == "Темно-синяя тема") {
        setDarkBlueTheme();
    } else if (theme == "Монохромная тема") {
        setMonochromeTheme();
    } else if (theme == "Пастельная тема") {
        setPastelTheme();
    } else if (theme == "Высококонтрастная тема") {
        setHighContrastTheme();
    } else if (theme == "Аналоговая цветовая схема") {
        setAnalogousColorScheme();
    } else if (theme == "Цветовая схема триада") {
        setTriadicColorScheme();
    } else if (theme == "Дополнительная цветовая схема") {
        setComplementaryColorScheme();
    } else if (theme == "Сплит-комплементарной") {
        setSplitComplementaryColorScheme();
    } else if (theme == "Креативные цветовые схемы") {
        setCreativeColorSchemes();
    }
}

void Settings::setDefaultTheme() {
    // Сброс стилей к стандартным настройкам Qt
    qApp->setStyleSheet("");
}

void Settings::setLightTheme() {
    QString styleSheet = R"(
        QWidget {
            background-color: #f0f0f0;
        }
        QPushButton {
            background-color: #007BFF; /* Измененный цвет кнопки */
            color: white;
            border-radius: 5px;
            padding: 10px;
        }
        QPushButton:hover {
            background-color: #0056b3; /* Измененный цвет кнопки при наведении */
        }
        QComboBox {
            background-color: white;
            border: 1px solid #ccc;
            border-radius: 5px;
        }
        QSpinBox {
            background-color: white;
            border: 1px solid #ccc;
            border-radius: 5px;
        }
        QLabel {
            color: black;
        }
        QTextEdit {
            background-color: white;
            border: 1px solid #ccc;
            border-radius: 5px;
        }
    )";
    qApp->setStyleSheet(styleSheet);
}

void Settings::setDarkTheme() {
    QString styleSheet = R"(
        QWidget {
            background-color: #2e2e2e;
        }
        QPushButton {
            background-color: #007BFF; /* Измененный цвет кнопки */
            color: white;
            border-radius: 5px;
            padding: 10px;
        }
        QPushButton:hover {
            background-color: #0056b3; /* Измененный цвет кнопки при наведении */
        }
        QComboBox {
            background-color: #444;
            color: white;
            border: 1px solid #666;
            border-radius: 5px;
        }
        QSpinBox {
            background-color: #444;
            color: white;
            border: 1px solid #666;
            border-radius: 5px;
        }
        QLabel {
            color: white;
        }
        QTextEdit {
            background-color: #444;
            color: white;
            border: 1px solid #666;
            border-radius: 5px;
        }
    )";
    qApp->setStyleSheet(styleSheet);
}

void Settings::setNightTheme() {
    QString styleSheet = R"(
        QWidget {
            background-color: #000000;
        }
        QPushButton {
            background-color: #1E90FF;
            color: white;
            border-radius: 5px;
            padding: 10px;
        }
        QPushButton:hover {
            background-color: #1C86EE;
        }
        QComboBox {
            background-color: #333333;
            color: white;
            border: 1px solid #666;
            border-radius: 5px;
        }
        QSpinBox {
            background-color: #333333;
            color: white;
            border: 1px solid #666;
            border-radius: 5px;
        }
        QLabel {
            color: white;
        }
        QTextEdit {
            background-color: #333333;
            color: white;
            border: 1px solid #666;
            border-radius: 5px;
        }
    )";
    qApp->setStyleSheet(styleSheet);
}

void Settings::setBlueTheme() {
    QString styleSheet = R"(
        QWidget {
            background-color: #e0f7fa;
        }
        QPushButton {
            background-color: #007BFF;
            color: white ;
            border-radius: 5px;
            padding: 10px;
        }
        QPushButton:hover {
            background-color: #0056b3;
        }
        QComboBox {
            background-color: #ffffff;
            border: 1px solid #007BFF;
            border-radius: 5px;
        }
        QSpinBox {
            background-color: #ffffff;
            border: 1px solid #007BFF;
            border-radius: 5px;
        }
        QLabel {
            color: #007BFF;
        }
        QTextEdit {
            background-color: #ffffff;
            border: 1px solid #007BFF;
            border-radius: 5px;
        }
    )";
    qApp->setStyleSheet(styleSheet);
}

void Settings::setGreenTheme() {
    QString styleSheet = R"(
        QWidget {
            background-color: #e8f5e9;
        }
        QPushButton {
            background-color: #4CAF50;
            color: white;
            border-radius: 5px;
            padding: 10px;
        }
        QPushButton:hover {
            background-color: #388E3C;
        }
        QComboBox {
            background-color: #ffffff;
            border: 1px solid #4CAF50;
            border-radius: 5px;
        }
        QSpinBox {
            background-color: #ffffff;
            border: 1px solid #4CAF50;
            border-radius: 5px;
        }
        QLabel {
            color: #4CAF50;
        }
        QTextEdit {
            background-color: #ffffff;
            border: 1px solid #4CAF50;
            border-radius: 5px;
        }
    )";
    qApp->setStyleSheet(styleSheet);
}

void Settings::setPinkTheme() {
    QString styleSheet = R"(
        QWidget {
            background-color: #fce4ec;
        }
        QPushButton {
            background-color: #D81B60;
            color: white;
            border-radius: 5px;
            padding: 10px;
        }
        QPushButton:hover {
            background-color: #C2185B;
        }
        QComboBox {
            background-color: #ffffff;
            border: 1px solid #D81B60;
            border-radius: 5px;
        }
        QSpinBox {
            background-color: #ffffff;
            border: 1px solid #D81B60;
            border-radius: 5px;
        }
        QLabel {
            color: #D81B60;
        }
        QTextEdit {
            background-color: #ffffff;
            border: 1px solid #D81B60;
            border-radius: 5px;
        }
    )";
    qApp->setStyleSheet(styleSheet);
}

void Settings::setDarkBlueTheme() {
    QString styleSheet = R"(
        QWidget {
            background-color: #001F3F;
        }
        QPushButton {
            background-color: #007BFF;
            color: white;
            border-radius: 5px;
            padding: 10px;
        }
        QPushButton:hover {
            background-color: #0056b3;
        }
        QComboBox {
            background-color: #003366;
            color: white;
            border: 1px solid #007BFF;
            border-radius: 5px;
        }
        QSpinBox {
            background-color: #003366;
            color: white;
            border: 1px solid #007BFF;
            border-radius: 5px;
        }
        QLabel {
            color: white;
        }
        QTextEdit {
            background-color: #003366;
            color: white;
            border: 1px solid #007BFF;
            border-radius: 5px;
        }
    )";
    qApp->setStyleSheet(styleSheet);
}

void Settings::setMonochromeTheme() {
    QString styleSheet = R"(
        QWidget {
            background-color: #FFFFFF;
        }
        QPushButton {
            background-color: #000000;
            color: white;
            border-radius: 5px;
            padding: 10px;
        }
        QPushButton:hover {
            background-color: #333333;
        }
        QComboBox {
            background-color: #FFFFFF;
            border: 1px solid #000000;
            border-radius: 5px;
        }
        QSpinBox {
            background-color: #FFFFFF;
            border: 1px solid #000000;
            border-radius: 5px;
        }
        QLabel {
            color: black;
        }
        QTextEdit {
            background-color: #FFFFFF;
            border: 1px solid #000000;
            border-radius: 5px;
        }
    )";
    qApp ->setStyleSheet(styleSheet);
}

void Settings::setPastelTheme() {
    QString styleSheet = R"(
        QWidget {
            background-color: #F8BBD0;
        }
        QPushButton {
            background-color: #FFABAB;
            color: white;
            border-radius: 5px;
            padding: 10px;
        }
        QPushButton:hover {
            background-color: #FF6F61;
        }
        QComboBox {
            background-color: #FFFFFF;
            border: 1px solid #FFABAB;
            border-radius: 5px;
        }
        QSpinBox {
            background-color: #FFFFFF;
            border: 1px solid #FFABAB;
            border-radius: 5px;
        }
        QLabel {
            color: #FFABAB;
        }
        QTextEdit {
            background-color: #FFFFFF;
            border: 1px solid #FFABAB;
            border-radius: 5px;
        }
    )";
    qApp->setStyleSheet(styleSheet);
}

void Settings::setHighContrastTheme() {
    QString styleSheet = R"(
        QWidget {
            background-color: #000000;
        }
        QPushButton {
            background-color: #FF0000;
            color: white;
            border-radius: 5px;
            padding: 10px;
        }
        QPushButton:hover {
            background-color: #FF4C4C;
        }
        QComboBox {
            background-color: #FFFFFF;
            border: 1px solid #FF0000;
            border-radius: 5px;
        }
        QSpinBox {
            background-color: #FFFFFF;
            border: 1px solid #FF0000;
            border-radius: 5px;
        }
        QLabel {
            color: #FFFFFF;
        }
        QTextEdit {
            background-color: #FFFFFF;
            border: 1px solid #FF0000;
            border-radius: 5px;
        }
    )";
    qApp->setStyleSheet(styleSheet);
}

void Settings::setAnalogousColorScheme() {
    QString styleSheet = R"(
        QWidget {
            background-color: #E0F7FA;
        }
        QPushButton {
            background-color: #4DB6AC;
            color: white;
            border-radius: 5px;
            padding: 10px;
        }
        QPushButton:hover {
            background-color: #009688;
        }
        QComboBox {
            background-color: #FFFFFF;
            border: 1px solid #4DB6AC;
            border-radius: 5px;
        }
        QSpinBox {
            background-color: #FFFFFF;
            border: 1px solid #4DB6AC;
            border-radius: 5px;
        }
        QLabel {
            color: #00796B;
        }
        QTextEdit {
            background-color: #FFFFFF;
            border: 1px solid #4DB6AC;
            border-radius: 5px;
        }
    )";
    qApp->setStyleSheet(styleSheet);
}

void Settings::setTriadicColorScheme() {
    QString styleSheet = R"(
        QWidget {
            background-color: #FCE4EC;
        }
        QPushButton {
            background-color: #D81B60;
            color: white;
            border-radius: 5px;
            padding: 10px;
        }
        QPushButton:hover {
            background-color: #C2185B;
        }
        QComboBox {
            background-color: #FFFFFF;
            border: 1px solid #D81B60;
            border-radius: 5px;
        }
        QSpinBox {
            background-color: #FFFFFF;
            border: 1px solid #D81B60;
            border-radius: 5px;
        }
        QLabel {
            color: #D81B60;
        }
        QTextEdit {
            background-color: #FFFFFF;
            border: 1px solid #D81B60;
            border-radius: 5px;
        }
    )";
    qApp->setStyleSheet(styleSheet);
}

void Settings::setComplementaryColorScheme() {
    QString styleSheet = R"(
        QWidget {
            background-color: #FFFFFF;
        }
        QPushButton {
            background-color: #FF5722;
            color: white;
            border-radius: 5px;
            padding: 10px;
        }
        QPushButton:hover {
            background-color: #E64A19;
        }
        QComboBox {
            background-color: #FFFFFF;
            border: 1px solid #FF5722;
            border-radius: 5px;
        }
        QSpinBox {
            background-color: #FFFFFF;
            border: 1px solid #FF5722;
            border-radius: 5px;
        }
        QLabel {
            color: #FF5722;
        }
        QTextEdit {
            background-color: #FFFFFF;
            border: 1px solid #FF5722;
            border-radius: 5px;
        }
    )";
    qApp->setStyleSheet(styleSheet);
}

void Settings::setSplitComplementaryColorScheme() {
    QString styleSheet = R"(
        QWidget {
            background-color: #F1F ```cpp
F1F1;
        }
        QPushButton {
            background-color: #FF9800;
            color: white;
            border-radius: 5px;
            padding: 10px;
        }
        QPushButton:hover {
            background-color: #FB8C00;
        }
        QComboBox {
            background-color: #FFFFFF;
            border: 1px solid #FF9800;
            border-radius: 5px;
        }
        QSpinBox {
            background-color: #FFFFFF;
            border: 1px solid #FF9800;
            border-radius: 5px;
        }
        QLabel {
            color: #FF9800;
        }
        QTextEdit {
            background-color: #FFFFFF;
            border: 1px solid #FF9800;
            border-radius: 5px;
        }
    )";
    qApp->setStyleSheet(styleSheet);
}

void Settings::setCreativeColorSchemes() {
    QString styleSheet = R"(
        QWidget {
            background-color: #E1BEE7;
        }
        QPushButton {
            background-color: #8E24AA;
            color: white;
            border-radius: 5px;
            padding: 10px;
        }
        QPushButton:hover {
            background-color: #7B1FA2;
        }
        QComboBox {
            background-color: #FFFFFF;
            border: 1px solid #8E24AA;
            border-radius: 5px;
        }
        QSpinBox {
            background-color: #FFFFFF;
            border: 1px solid #8E24AA;
            border-radius: 5px;
        }
        QLabel {
            color: #8E24AA;
        }
        QTextEdit {
            background-color: #FFFFFF;
            border: 1px solid #8E24AA;
            border-radius: 5px;
        }
    )";
    qApp->setStyleSheet(styleSheet);
}


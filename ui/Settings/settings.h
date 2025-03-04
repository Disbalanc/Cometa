#ifndef SETTINGS_H
#define SETTINGS_H

#include <QComboBox>
#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QTranslator> // Добавлено для использования QTranslator

namespace Ui {
class Settings;
}

class Settings : public QDialog
{
    Q_OBJECT

public:
    explicit Settings(QWidget *parent = nullptr);
    ~Settings();
    static void applyTheme(const QString& theme);
    static void applyLanguage(const QString& lang);
    static void applyFontSize(int size);

signals:
    void databasePathChanged(const QString &dbPath);
    void themeChanged(const QString &theme);

private slots:
    void onThemeChanged(const QString &theme);
    void onFontSizeChanged(int size);
    void onLanguageChanged(const QString &language);

private:

    void loadSettings();
    void saveSettingsDataBase();
    void saveSettingsTheme();
    void saveSettingsLanguage();
    void saveSettingsFontSize();

    void onButtonRadiusChanged(int radius);
    void onButtonPaddingChanged(int padding);

    static void setDefaultTheme();
    static void setLightTheme();
    static void setDarkTheme();
    static void setNightTheme();
    static void setBlueTheme();
    static void setGreenTheme();
    static void setPinkTheme();
    static void setDarkBlueTheme();
    static void setMonochromeTheme();
    static void setPastelTheme();
    static void setHighContrastTheme();
    static void setAnalogousColorScheme();
    static void setTriadicColorScheme();
    static void setComplementaryColorScheme();
    static void setSplitComplementaryColorScheme();
    static void setCreativeColorSchemes();

    QLineEdit *dbPathLineEdit;
    QComboBox *themeComboBox;
    QComboBox *languageComboBox;
    QSpinBox *fontSizeSpinBox;
    Ui::Settings *ui;
    QTranslator *translator; // Указатель на QTranslator для управления переводами
};

#endif // SETTINGS_H

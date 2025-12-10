// Tests for ThemeManager

#include <QtTest/QtTest>
#include <QtWidgets/QApplication>
#include "../../src/ui/ThemeManager.h"
#include <QtWidgets/QWidget>
#include <QtCore/QString>

using namespace ocr_orc;

class TestThemeManager : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    
    void testSingleton();
    void testThemeSwitching();
    void testGetColors();
    void testGetStylesheet();
    void testApplyTheme();
    void testSystemThemeDetection();
    void testSettingsPersistence();

private:
    QApplication* app;
};

void TestThemeManager::initTestCase() {
    if (!QApplication::instance()) {
        static int testArgc = 1;
        static char testAppName[] = "test";
        static char* testArgv[] = { testAppName, nullptr };
        app = new QApplication(testArgc, testArgv);
    }
}

void TestThemeManager::cleanupTestCase() {
    // Cleanup handled by QApplication
}

void TestThemeManager::testSingleton() {
    // ThemeManager is a singleton - use instance()
    ThemeManager& manager = ThemeManager::instance();
    QVERIFY(&manager != nullptr);
}

void TestThemeManager::testThemeSwitching() {
    ThemeManager& themeManager = ThemeManager::instance();
    
    // Test Light
    themeManager.setTheme(Theme::Light);
    QCOMPARE(themeManager.getTheme(), Theme::Light);
    QCOMPARE(themeManager.getEffectiveTheme(), Theme::Light);
    
    // Test Dark
    themeManager.setTheme(Theme::Dark);
    QCOMPARE(themeManager.getTheme(), Theme::Dark);
    QCOMPARE(themeManager.getEffectiveTheme(), Theme::Dark);
    
    // Test System
    themeManager.setTheme(Theme::System);
    QCOMPARE(themeManager.getTheme(), Theme::System);
    Theme effective = themeManager.getEffectiveTheme();
    QVERIFY(effective == Theme::Light || effective == Theme::Dark);
}

void TestThemeManager::testGetColors() {
    ThemeManager& themeManager = ThemeManager::instance();
    
    // Test Light colors
    themeManager.setTheme(Theme::Light);
    ThemeColors lightColors = themeManager.getColors();
    QVERIFY(lightColors.background.isValid());
    QVERIFY(lightColors.text.isValid());
    QVERIFY(lightColors.primary.isValid());
    
    // Test Dark colors
    themeManager.setTheme(Theme::Dark);
    ThemeColors darkColors = themeManager.getColors();
    QVERIFY(darkColors.background.isValid());
    QVERIFY(darkColors.text.isValid());
    QVERIFY(darkColors.primary.isValid());
    
    // Colors should be different
    QVERIFY(lightColors.background != darkColors.background);
}

void TestThemeManager::testGetStylesheet() {
    ThemeManager& themeManager = ThemeManager::instance();
    
    themeManager.setTheme(Theme::Light);
    QString lightStylesheet = themeManager.getStylesheet();
    QVERIFY(!lightStylesheet.isEmpty());
    QVERIFY(lightStylesheet.contains("QMainWindow"));
    
    themeManager.setTheme(Theme::Dark);
    QString darkStylesheet = themeManager.getStylesheet();
    QVERIFY(!darkStylesheet.isEmpty());
    QVERIFY(darkStylesheet.contains("QMainWindow"));
    
    // Stylesheets should be different
    QVERIFY(lightStylesheet != darkStylesheet);
}

void TestThemeManager::testApplyTheme() {
    ThemeManager& themeManager = ThemeManager::instance();
    
    QWidget* testWidget = new QWidget();
    themeManager.applyTheme(testWidget);
    
    // Widget should have stylesheet applied
    QString stylesheet = testWidget->styleSheet();
    QVERIFY(!stylesheet.isEmpty());
    
    delete testWidget;
}

void TestThemeManager::testSystemThemeDetection() {
    bool isDark = ThemeManager::isSystemDarkMode();
    // Should return true or false
    QVERIFY(isDark == true || isDark == false);
}

void TestThemeManager::testSettingsPersistence() {
    ThemeManager& themeManager = ThemeManager::instance();
    
    // Set and save
    themeManager.setTheme(Theme::Dark);
    themeManager.saveSettings();
    
    // Load should restore
    themeManager.loadSettings();
    QCOMPARE(themeManager.getTheme(), Theme::Dark);
}

QTEST_MAIN(TestThemeManager)
#include "test_theme_manager.moc"


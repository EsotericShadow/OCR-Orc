// Test file for InputValidator
// Tests input validation functionality

#include <QtTest/QtTest>
#include "../src/utils/InputValidator.h"
#include "../src/core/CoordinateSystem.h"

using namespace ocr_orc;

class TestInputValidator : public QObject {
    Q_OBJECT

private slots:
    void testValidateRegionName();
    void testValidateRegionNameEmpty();
    void testValidateRegionNameDuplicate();
    void testValidateGroupName();
    void testValidateGroupNameDuplicate();
    void testValidateNormalizedCoords();
    void testValidateNormalizedCoordsInvalid();
    void testValidateFilePath();
    void testValidateDpi();
    void testSanitizeRegionName();
    void testSanitizeGroupName();
    void testSanitizeFilePath();
};

void TestInputValidator::testValidateRegionName() {
    QString result = InputValidator::validateRegionName("ValidRegion");
    QVERIFY(result.isEmpty());
    
    result = InputValidator::validateRegionName("Region123");
    QVERIFY(result.isEmpty());
    
    result = InputValidator::validateRegionName("Region_Name");
    QVERIFY(result.isEmpty());
}

void TestInputValidator::testValidateRegionNameEmpty() {
    QString result = InputValidator::validateRegionName("");
    QVERIFY(!result.isEmpty());
    QVERIFY(result.contains("empty", Qt::CaseInsensitive));
}

void TestInputValidator::testValidateRegionNameDuplicate() {
    QList<QString> existing;
    existing << "ExistingRegion" << "AnotherRegion";
    
    QString result = InputValidator::validateRegionName("ExistingRegion", existing);
    QVERIFY(!result.isEmpty());
    QVERIFY(result.contains("already", Qt::CaseInsensitive) || 
            result.contains("duplicate", Qt::CaseInsensitive) ||
            result.contains("exists", Qt::CaseInsensitive));
    
    result = InputValidator::validateRegionName("NewRegion", existing);
    QVERIFY(result.isEmpty());
}

void TestInputValidator::testValidateGroupName() {
    QString result = InputValidator::validateGroupName("ValidGroup");
    QVERIFY(result.isEmpty());
    
    result = InputValidator::validateGroupName("Group123");
    QVERIFY(result.isEmpty());
}

void TestInputValidator::testValidateGroupNameDuplicate() {
    QList<QString> existing;
    existing << "ExistingGroup";
    
    QString result = InputValidator::validateGroupName("ExistingGroup", existing);
    QVERIFY(!result.isEmpty());
    
    result = InputValidator::validateGroupName("NewGroup", existing);
    QVERIFY(result.isEmpty());
}

void TestInputValidator::testValidateNormalizedCoords() {
    NormalizedCoords valid(0.25, 0.10, 0.35, 0.20);
    QString result = InputValidator::validateNormalizedCoords(valid);
    QVERIFY(result.isEmpty());
    
    NormalizedCoords valid2(0.0, 0.0, 1.0, 1.0);
    result = InputValidator::validateNormalizedCoords(valid2);
    QVERIFY(result.isEmpty());
}

void TestInputValidator::testValidateNormalizedCoordsInvalid() {
    NormalizedCoords invalid1(1.5, 0.10, 0.35, 0.20); // x1 out of range
    QString result = InputValidator::validateNormalizedCoords(invalid1);
    QVERIFY(!result.isEmpty());
    
    NormalizedCoords invalid2(0.35, 0.10, 0.25, 0.20); // x1 > x2
    result = InputValidator::validateNormalizedCoords(invalid2);
    QVERIFY(!result.isEmpty());
}

void TestInputValidator::testValidateFilePath() {
    QString result = InputValidator::validateFilePath("/valid/path/file.pdf");
    QVERIFY(result.isEmpty());
    
    result = InputValidator::validateFilePath("relative/path.pdf");
    QVERIFY(result.isEmpty());
}

void TestInputValidator::testValidateDpi() {
    QString result = InputValidator::validateDpi(150);
    QVERIFY(result.isEmpty());
    
    result = InputValidator::validateDpi(300);
    QVERIFY(result.isEmpty());
    
    result = InputValidator::validateDpi(50);
    QVERIFY(!result.isEmpty());
    
    result = InputValidator::validateDpi(1000);
    QVERIFY(!result.isEmpty());
}

void TestInputValidator::testSanitizeRegionName() {
    QString sanitized = InputValidator::sanitizeRegionName("Test/Region\\Name");
    QVERIFY(!sanitized.contains("/"));
    QVERIFY(!sanitized.contains("\\"));
    
    sanitized = InputValidator::sanitizeRegionName("NormalName");
    QCOMPARE(sanitized, QString("NormalName"));
}

void TestInputValidator::testSanitizeGroupName() {
    QString sanitized = InputValidator::sanitizeGroupName("Test/Group");
    QVERIFY(!sanitized.contains("/"));
    
    sanitized = InputValidator::sanitizeGroupName("NormalGroup");
    QCOMPARE(sanitized, QString("NormalGroup"));
}

void TestInputValidator::testSanitizeFilePath() {
    QString sanitized = InputValidator::sanitizeFilePath("../../../etc/passwd");
    // Should prevent path traversal
    QVERIFY(sanitized.isEmpty() || !sanitized.contains("../"));
    
    sanitized = InputValidator::sanitizeFilePath("/valid/path/file.pdf");
    QVERIFY(!sanitized.isEmpty());
}

QTEST_MAIN(TestInputValidator)
#include "test_input_validator.moc"


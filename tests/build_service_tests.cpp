#include "platform/BuildService.hpp"

#include <QtTest/QtTest>

using MalloyWriter::Platform::BuildService;
using MalloyWriter::Platform::DiagnosticSeverity;

class BuildServiceTests : public QObject {
    Q_OBJECT

private slots:
    void parsesGccDiagnostics()
    {
        const QString output = "E:/Project/main.cpp:12:7: error: expected ';' before 'return'\n"
                               "E:/Project/main.cpp:18:2: warning: unused variable 'x'\n";

        const auto diagnostics = BuildService::parseDiagnostics(output);
        QCOMPARE(diagnostics.size(), 2);
        QCOMPARE(diagnostics[0].filePath, QString("E:/Project/main.cpp"));
        QCOMPARE(diagnostics[0].line, 12);
        QCOMPARE(diagnostics[0].column, 7);
        QCOMPARE(diagnostics[0].severity, DiagnosticSeverity::Error);
        QCOMPARE(diagnostics[1].severity, DiagnosticSeverity::Warning);
    }
};

QTEST_MAIN(BuildServiceTests)

#include "build_service_tests.moc"

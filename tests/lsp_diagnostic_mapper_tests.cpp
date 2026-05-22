#include "languages/LspDiagnosticMapper.hpp"

#include <QJsonArray>
#include <QJsonObject>
#include <QtTest/QtTest>

using MalloyWriter::Languages::diagnosticsFromLsp;
using MalloyWriter::Platform::DiagnosticSeverity;

namespace {

QJsonObject makeDiagnostic(int line, int character, int severity, const QString &message)
{
    QJsonObject start;
    start[QStringLiteral("line")] = line;
    start[QStringLiteral("character")] = character;
    QJsonObject range;
    range[QStringLiteral("start")] = start;
    QJsonObject diagnostic;
    diagnostic[QStringLiteral("range")] = range;
    diagnostic[QStringLiteral("severity")] = severity;
    diagnostic[QStringLiteral("message")] = message;
    return diagnostic;
}

} // namespace

class LspDiagnosticMapperTests : public QObject {
    Q_OBJECT

private slots:
    void mapsPositionMessageAndSource()
    {
        QJsonArray array;
        array.append(makeDiagnostic(9, 4, 1, QStringLiteral("undeclared identifier")));

        const auto result = diagnosticsFromLsp(QStringLiteral("file:///C:/proj/main.cpp"), array);
        QCOMPARE(result.size(), 1);
        QCOMPARE(result.at(0).line, 10);   // LSP 0-based -> editor 1-based
        QCOMPARE(result.at(0).column, 5);
        QCOMPARE(result.at(0).message, QString("undeclared identifier"));
        QCOMPARE(result.at(0).source, QString("clangd"));
        QVERIFY(result.at(0).filePath.endsWith(QStringLiteral("main.cpp")));
        QVERIFY(result.at(0).severity == DiagnosticSeverity::Error);
    }

    void mapsSeverityVariants()
    {
        auto severityFor = [](int severity) {
            QJsonArray array;
            array.append(makeDiagnostic(0, 0, severity, QStringLiteral("m")));
            return diagnosticsFromLsp(QStringLiteral("file:///x.cpp"), array).at(0).severity;
        };
        QVERIFY(severityFor(1) == DiagnosticSeverity::Error);
        QVERIFY(severityFor(2) == DiagnosticSeverity::Warning);
        QVERIFY(severityFor(3) == DiagnosticSeverity::Note);
        QVERIFY(severityFor(4) == DiagnosticSeverity::Note);
    }

    void emptyArrayYieldsNoDiagnostics()
    {
        QCOMPARE(diagnosticsFromLsp(QStringLiteral("file:///x.cpp"), QJsonArray{}).size(), 0);
    }
};

QTEST_MAIN(LspDiagnosticMapperTests)

#include "lsp_diagnostic_mapper_tests.moc"

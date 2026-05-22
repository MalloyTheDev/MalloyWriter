#include "languages/LspDiagnosticMapper.hpp"

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QUrl>

namespace MalloyWriter::Languages {

using MalloyWriter::Platform::Diagnostic;
using MalloyWriter::Platform::DiagnosticSeverity;

QList<Diagnostic> diagnosticsFromLsp(const QString &uri, const QJsonArray &lspDiagnostics)
{
    const QString filePath = QUrl(uri).toLocalFile();
    QList<Diagnostic> result;
    result.reserve(lspDiagnostics.size());

    for (const QJsonValue &value : lspDiagnostics) {
        const QJsonObject object = value.toObject();
        const QJsonObject start = object.value(QStringLiteral("range")).toObject()
                                       .value(QStringLiteral("start")).toObject();

        Diagnostic diagnostic;
        diagnostic.filePath = filePath;
        diagnostic.line = start.value(QStringLiteral("line")).toInt() + 1;        // LSP is 0-based
        diagnostic.column = start.value(QStringLiteral("character")).toInt() + 1;
        diagnostic.message = object.value(QStringLiteral("message")).toString();
        diagnostic.source = QStringLiteral("clangd");

        switch (object.value(QStringLiteral("severity")).toInt(1)) {
        case 1:  diagnostic.severity = DiagnosticSeverity::Error; break;
        case 2:  diagnostic.severity = DiagnosticSeverity::Warning; break;
        default: diagnostic.severity = DiagnosticSeverity::Note; break;
        }

        result.append(diagnostic);
    }
    return result;
}

} // namespace MalloyWriter::Languages

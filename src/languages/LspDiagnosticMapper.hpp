#pragma once

#include "platform/Diagnostic.hpp"

#include <QList>
#include <QString>

class QJsonArray;

namespace MalloyWriter::Languages {

// Convert an LSP textDocument/publishDiagnostics array (for one document URI)
// into the platform Diagnostic model. LSP positions are 0-based; the editor
// model is 1-based. The source field is tagged "clangd".
QList<MalloyWriter::Platform::Diagnostic> diagnosticsFromLsp(const QString &uri,
                                                             const QJsonArray &lspDiagnostics);

} // namespace MalloyWriter::Languages

#pragma once

#include <QMetaType>
#include <QString>

namespace MalloyWriter::Platform {

enum class DiagnosticSeverity {
    Note,
    Warning,
    Error
};

struct Diagnostic {
    QString filePath;
    int line = 0;
    int column = 0;
    DiagnosticSeverity severity = DiagnosticSeverity::Note;
    QString message;
    QString source;
};

}

Q_DECLARE_METATYPE(MalloyWriter::Platform::Diagnostic)

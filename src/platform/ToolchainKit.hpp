#pragma once

#include <QString>
#include <QStringList>

namespace MalloyWriter::Platform {

struct ToolchainKit {
    QString id;
    QString displayName;
    QString cmakeProgram;
    QString generator;
    QString makeProgram;
    QString cCompiler;
    QString cxxCompiler;
    QString clangdProgram;
    QString debuggerProgram;

    bool isUsable() const;
    QStringList missingTools() const;
};

class ToolchainKitDetector {
public:
    static ToolchainKit defaultWindowsKit();
    static QList<ToolchainKit> detectAvailableKits();
};

}

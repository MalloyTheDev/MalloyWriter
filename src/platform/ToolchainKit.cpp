#include "platform/ToolchainKit.hpp"

#include <QFileInfo>
#include <QStandardPaths>

namespace MalloyWriter::Platform {
namespace {

QString firstExisting(const QStringList &candidates)
{
    for (const QString &candidate : candidates) {
        if (QFileInfo::exists(candidate)) {
            return candidate;
        }
        const QString resolved = QStandardPaths::findExecutable(candidate);
        if (!resolved.isEmpty()) {
            return resolved;
        }
    }
    return {};
}

bool existsOrOnPath(const QString &path)
{
    return QFileInfo::exists(path) || !QStandardPaths::findExecutable(path).isEmpty();
}

}

bool ToolchainKit::isUsable() const
{
    return missingTools().isEmpty();
}

QStringList ToolchainKit::missingTools() const
{
    QStringList missing;
    const QList<QPair<QString, QString>> tools = {
        {"CMake", cmakeProgram},
        {"Build tool", makeProgram},
        {"C compiler", cCompiler},
        {"C++ compiler", cxxCompiler},
        {"clangd", clangdProgram},
        {"debugger", debuggerProgram}
    };

    for (const auto &tool : tools) {
        if (!existsOrOnPath(tool.second)) {
            missing << tool.first;
        }
    }
    return missing;
}

ToolchainKit ToolchainKitDetector::defaultWindowsKit()
{
    return {
        "msys2-mingw64",
        "MSYS2 MinGW64 GCC + LLVM clangd",
        firstExisting({"C:/Program Files/CMake/bin/cmake.exe", "cmake"}),
        "Ninja",
        firstExisting({"C:/ProgramData/chocolatey/bin/ninja.exe", "C:/msys64/mingw64/bin/ninja.exe", "ninja"}),
        firstExisting({"C:/msys64/mingw64/bin/gcc.exe", "gcc"}),
        firstExisting({"C:/msys64/mingw64/bin/g++.exe", "g++"}),
        firstExisting({"C:/Program Files/LLVM/bin/clangd.exe", "C:/msys64/mingw64/bin/clangd.exe", "clangd"}),
        firstExisting({"C:/msys64/mingw64/bin/gdb.exe", "gdb"})
    };
}

QList<ToolchainKit> ToolchainKitDetector::detectAvailableKits()
{
    QList<ToolchainKit> kits;
    kits << defaultWindowsKit();
    return kits;
}

}

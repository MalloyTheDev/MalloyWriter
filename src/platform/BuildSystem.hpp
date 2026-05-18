#pragma once

#include <QString>
#include <QStringList>

namespace MalloyWriter::Platform {

struct BuildSettings {
    QString cmakeProgram = "C:/Program Files/CMake/bin/cmake.exe";
    QString generator = "Ninja";
    QString buildDirectory = ".malloywriter/build";
    QString makeProgram = "C:/ProgramData/chocolatey/bin/ninja.exe";
    QString cCompiler = "C:/msys64/mingw64/bin/gcc.exe";
    QString cxxCompiler = "C:/msys64/mingw64/bin/g++.exe";
};

struct BuildCommand {
    QString program;
    QStringList arguments;
    QString workingDirectory;
};

class CMakeBuildPlanner {
public:
    static QString resolvedBuildDirectory(const QString &sourceDirectory, const BuildSettings &settings);
    static BuildCommand configureCommand(const QString &sourceDirectory, const BuildSettings &settings);
    static BuildCommand buildCommand(const QString &sourceDirectory, const BuildSettings &settings, const QString &target = {});
};

BuildSettings defaultBuildSettings();

}

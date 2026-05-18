#include "platform/BuildSystem.hpp"

#include <QDir>
#include <QFileInfo>

namespace MalloyWriter::Platform {

namespace {

QString absolutePathFrom(const QString &baseDirectory, const QString &path)
{
    const QFileInfo info(path);
    if (info.isAbsolute()) {
        return QDir::cleanPath(info.absoluteFilePath());
    }
    return QDir::cleanPath(QDir(baseDirectory).absoluteFilePath(path));
}

}

QString CMakeBuildPlanner::resolvedBuildDirectory(const QString &sourceDirectory, const BuildSettings &settings)
{
    return absolutePathFrom(sourceDirectory, settings.buildDirectory);
}

BuildCommand CMakeBuildPlanner::configureCommand(const QString &sourceDirectory, const BuildSettings &settings)
{
    QStringList arguments = {
        "-S", QDir::cleanPath(QFileInfo(sourceDirectory).absoluteFilePath()),
        "-B", resolvedBuildDirectory(sourceDirectory, settings),
        "-G", settings.generator
    };

    if (!settings.makeProgram.trimmed().isEmpty()) {
        arguments << QString("-DCMAKE_MAKE_PROGRAM=%1").arg(settings.makeProgram);
    }
    if (!settings.cCompiler.trimmed().isEmpty()) {
        arguments << QString("-DCMAKE_C_COMPILER=%1").arg(settings.cCompiler);
    }
    if (!settings.cxxCompiler.trimmed().isEmpty()) {
        arguments << QString("-DCMAKE_CXX_COMPILER=%1").arg(settings.cxxCompiler);
    }

    return {settings.cmakeProgram, arguments, QDir::cleanPath(QFileInfo(sourceDirectory).absoluteFilePath())};
}

BuildCommand CMakeBuildPlanner::buildCommand(const QString &sourceDirectory, const BuildSettings &settings, const QString &target)
{
    QStringList arguments = {"--build", resolvedBuildDirectory(sourceDirectory, settings)};
    if (!target.trimmed().isEmpty()) {
        arguments << "--target" << target.trimmed();
    }

    return {settings.cmakeProgram, arguments, QDir::cleanPath(QFileInfo(sourceDirectory).absoluteFilePath())};
}

BuildSettings defaultBuildSettings()
{
    return {};
}

}

#include "platform/SettingsService.hpp"

#include <QDir>
#include <QFileInfo>
#include <QSettings>

namespace MalloyWriter::Platform {

namespace {

constexpr int MaxRecentWorkspaces = 8;
constexpr int MaxRecentFiles = 12;

QString normalizedExistingPath(const QString &path)
{
    return QDir::cleanPath(QFileInfo(path).absoluteFilePath());
}

}

SettingsService::SettingsService(QObject *parent)
    : QObject(parent)
{
}

BuildSettings SettingsService::buildSettings() const
{
    QSettings settings;
    const BuildSettings defaults = defaultBuildSettings();

    BuildSettings result;
    result.cmakeProgram = settings.value("build/cmakeProgram", defaults.cmakeProgram).toString();
    result.generator = settings.value("build/generator", defaults.generator).toString();
    result.buildDirectory = settings.value("build/buildDirectory", defaults.buildDirectory).toString();
    result.makeProgram = settings.value("build/makeProgram", defaults.makeProgram).toString();
    result.cCompiler = settings.value("build/cCompiler", defaults.cCompiler).toString();
    result.cxxCompiler = settings.value("build/cxxCompiler", defaults.cxxCompiler).toString();
    return result;
}

void SettingsService::setBuildSettings(const BuildSettings &settingsValue)
{
    QSettings settings;
    settings.setValue("build/cmakeProgram", settingsValue.cmakeProgram);
    settings.setValue("build/generator", settingsValue.generator);
    settings.setValue("build/buildDirectory", settingsValue.buildDirectory);
    settings.setValue("build/makeProgram", settingsValue.makeProgram);
    settings.setValue("build/cCompiler", settingsValue.cCompiler);
    settings.setValue("build/cxxCompiler", settingsValue.cxxCompiler);
}

QStringList SettingsService::recentWorkspaces() const
{
    return listValue("recent/workspaces");
}

void SettingsService::addRecentWorkspace(const QString &path)
{
    addRecentPath("recent/workspaces", path, MaxRecentWorkspaces);
    emit recentWorkspacesChanged();
}

QStringList SettingsService::recentFiles() const
{
    return listValue("recent/files");
}

void SettingsService::addRecentFile(const QString &path)
{
    addRecentPath("recent/files", path, MaxRecentFiles);
    emit recentFilesChanged();
}

QString SettingsService::lastRunExecutable() const
{
    return QSettings().value("run/lastExecutable").toString();
}

void SettingsService::setLastRunExecutable(const QString &path)
{
    QSettings().setValue("run/lastExecutable", normalizedExistingPath(path));
}

QStringList SettingsService::listValue(const QString &key) const
{
    return QSettings().value(key).toStringList();
}

void SettingsService::addRecentPath(const QString &key, const QString &path, int maxItems)
{
    const QString normalized = normalizedExistingPath(path);
    if (normalized.trimmed().isEmpty()) {
        return;
    }

    QSettings settings;
    QStringList values = settings.value(key).toStringList();
    values.removeAll(normalized);
    values.prepend(normalized);
    while (values.size() > maxItems) {
        values.removeLast();
    }
    settings.setValue(key, values);
}

}

#include "platform/ProjectService.hpp"

#include <QDir>
#include <QFileInfo>

namespace MalloyWriter::Platform {

ProjectService::ProjectService(QObject *parent)
    : QObject(parent)
    , m_activeKit(ToolchainKitDetector::defaultWindowsKit())
{
}

bool ProjectService::openWorkspace(const QString &path)
{
    const QFileInfo info(path);
    if (!info.exists() || !info.isDir()) {
        return false;
    }

    const QString normalized = QDir::cleanPath(info.absoluteFilePath());
    if (m_workspaceRoot == normalized) {
        return true;
    }

    m_workspaceRoot = normalized;
    m_cmakeModel = {};
    emit workspaceChanged(m_workspaceRoot);
    emit cmakeModelChanged();
    return true;
}

void ProjectService::clear()
{
    if (m_workspaceRoot.isEmpty()) {
        return;
    }
    m_workspaceRoot.clear();
    m_cmakeModel = {};
    emit workspaceChanged({});
    emit cmakeModelChanged();
}

bool ProjectService::hasWorkspace() const
{
    return !m_workspaceRoot.isEmpty();
}

QString ProjectService::workspaceRoot() const
{
    return m_workspaceRoot;
}

QString ProjectService::buildDirectory() const
{
    return CMakeBuildPlanner::resolvedBuildDirectory(m_workspaceRoot, buildSettings());
}

ToolchainKit ProjectService::activeKit() const
{
    return m_activeKit;
}

BuildSettings ProjectService::buildSettings() const
{
    BuildSettings settings;
    settings.cmakeProgram = m_activeKit.cmakeProgram;
    settings.generator = m_activeKit.generator;
    settings.makeProgram = m_activeKit.makeProgram;
    settings.cCompiler = m_activeKit.cCompiler;
    settings.cxxCompiler = m_activeKit.cxxCompiler;
    return settings;
}

CMakeProjectModel ProjectService::cmakeModel() const
{
    return m_cmakeModel;
}

QStringList ProjectService::executableTargets() const
{
    QStringList targets;
    for (const CMakeTarget &target : m_cmakeModel.targets) {
        if (target.type == "EXECUTABLE") {
            targets << target.name;
        }
    }
    return targets;
}

bool ProjectService::prepareCMakeQuery(QString *errorMessage)
{
    if (!hasWorkspace()) {
        if (errorMessage) {
            *errorMessage = "No workspace is open.";
        }
        return false;
    }
    return CMakeFileApi::writeQuery(buildDirectory(), errorMessage);
}

bool ProjectService::refreshCMakeModel(QString *errorMessage)
{
    if (!hasWorkspace()) {
        if (errorMessage) {
            *errorMessage = "No workspace is open.";
        }
        return false;
    }

    QString localError;
    m_cmakeModel = CMakeFileApi::readModel(workspaceRoot(), buildDirectory(), &localError);
    emit cmakeModelChanged();
    if (!localError.isEmpty()) {
        if (errorMessage) {
            *errorMessage = localError;
        }
        return false;
    }
    return true;
}

void ProjectService::setActiveKit(const ToolchainKit &kit)
{
    m_activeKit = kit;
    emit kitChanged();
}

}

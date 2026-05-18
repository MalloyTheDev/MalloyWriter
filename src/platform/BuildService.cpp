#include "platform/BuildService.hpp"

#include <QFileInfo>
#include <QRegularExpression>

namespace MalloyWriter::Platform {
namespace {

DiagnosticSeverity severityFromText(const QString &text)
{
    if (text == "error" || text == "fatal error") {
        return DiagnosticSeverity::Error;
    }
    if (text == "warning") {
        return DiagnosticSeverity::Warning;
    }
    return DiagnosticSeverity::Note;
}

QString jobLabel(BuildJobKind kind)
{
    switch (kind) {
    case BuildJobKind::Configure:
        return "CMake configure";
    case BuildJobKind::Build:
        return "CMake build";
    case BuildJobKind::Test:
        return "CTest";
    case BuildJobKind::Run:
        return "Run executable";
    }
    return "Build job";
}

}

BuildService::BuildService(ProjectService *projectService, QObject *parent)
    : QObject(parent)
    , m_projectService(projectService)
{
    connect(&m_runner, &ProcessRunner::processStarted, this, &BuildService::jobStarted);
    connect(&m_runner, &ProcessRunner::outputReceived, this, [this](const QString &text) {
        emit outputReceived(text);
        const QList<Diagnostic> parsed = parseDiagnostics(text);
        if (!parsed.isEmpty()) {
            m_diagnostics.append(parsed);
            emit diagnosticsChanged(m_diagnostics);
        }
    });
    connect(&m_runner, &ProcessRunner::processFinished, this, [this](int exitCode, QProcess::ExitStatus status) {
        if (m_currentJob.kind == BuildJobKind::Configure && status == QProcess::NormalExit && exitCode == 0 && m_projectService) {
            QString ignored;
            m_projectService->refreshCMakeModel(&ignored);
        }
        emit jobFinished(m_currentJob.label, exitCode, status);
    });
}

bool BuildService::isRunning() const
{
    return m_runner.isRunning();
}

bool BuildService::configure()
{
    if (!m_projectService || !m_projectService->hasWorkspace()) {
        return false;
    }

    QString errorMessage;
    if (!m_projectService->prepareCMakeQuery(&errorMessage)) {
        emit outputReceived(errorMessage + '\n');
        return false;
    }

    BuildSettings settings = m_projectService->buildSettings();
    BuildCommand command = CMakeBuildPlanner::configureCommand(m_projectService->workspaceRoot(), settings);
    command.arguments << "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON";
    return startJob({BuildJobKind::Configure, jobLabel(BuildJobKind::Configure), command});
}

bool BuildService::build(const QString &target)
{
    if (!m_projectService || !m_projectService->hasWorkspace()) {
        return false;
    }
    return startJob({BuildJobKind::Build, jobLabel(BuildJobKind::Build), CMakeBuildPlanner::buildCommand(m_projectService->workspaceRoot(), m_projectService->buildSettings(), target)});
}

bool BuildService::test()
{
    if (!m_projectService || !m_projectService->hasWorkspace()) {
        return false;
    }

    const BuildSettings settings = m_projectService->buildSettings();
    const BuildCommand command{
        settings.cmakeProgram,
        {"--build", CMakeBuildPlanner::resolvedBuildDirectory(m_projectService->workspaceRoot(), settings), "--target", "test"},
        m_projectService->workspaceRoot()
    };
    return startJob({BuildJobKind::Test, jobLabel(BuildJobKind::Test), command});
}

bool BuildService::runExecutable(const QString &path)
{
    const QFileInfo executable(path);
    if (!executable.exists() || !executable.isFile()) {
        return false;
    }

    return startJob({BuildJobKind::Run, QString("Run %1").arg(executable.fileName()), {executable.absoluteFilePath(), {}, executable.absolutePath()}});
}

void BuildService::stop()
{
    m_runner.stop();
}

QList<Diagnostic> BuildService::diagnostics() const
{
    return m_diagnostics;
}

QList<Diagnostic> BuildService::parseDiagnostics(const QString &output)
{
    static const QRegularExpression diagnosticPattern(
        R"((?<file>(?:[A-Za-z]:)?[^:\r\n]+):(?<line>\d+):(?:(?<column>\d+):)?\s*(?<severity>fatal error|error|warning|note):\s*(?<message>[^\r\n]+))");

    QList<Diagnostic> diagnostics;
    const auto matches = diagnosticPattern.globalMatch(output);
    auto iterator = matches;
    while (iterator.hasNext()) {
        const QRegularExpressionMatch match = iterator.next();
        diagnostics << Diagnostic{
            match.captured("file"),
            match.captured("line").toInt(),
            match.captured("column").isEmpty() ? 0 : match.captured("column").toInt(),
            severityFromText(match.captured("severity")),
            match.captured("message").trimmed(),
            "compiler"
        };
    }
    return diagnostics;
}

bool BuildService::startJob(const BuildJob &job)
{
    if (m_runner.isRunning()) {
        return false;
    }

    m_currentJob = job;
    m_diagnostics.clear();
    emit diagnosticsChanged(m_diagnostics);
    const bool started = m_runner.start({job.label, job.command.program, job.command.arguments, job.command.workingDirectory});
    if (!started) {
        emit jobFailedToStart(job.label);
    }
    return started;
}

}

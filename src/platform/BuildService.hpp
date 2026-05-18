#pragma once

#include "platform/Diagnostic.hpp"
#include "platform/ProcessRunner.hpp"
#include "platform/ProjectService.hpp"

#include <QObject>

namespace MalloyWriter::Platform {

enum class BuildJobKind {
    Configure,
    Build,
    Test,
    Run
};

struct BuildJob {
    BuildJobKind kind = BuildJobKind::Build;
    QString label;
    BuildCommand command;
};

class BuildService : public QObject {
    Q_OBJECT

public:
    explicit BuildService(ProjectService *projectService, QObject *parent = nullptr);

    bool isRunning() const;
    bool configure();
    bool build(const QString &target = {});
    bool test();
    bool runExecutable(const QString &path);
    void stop();

    QList<Diagnostic> diagnostics() const;
    static QList<Diagnostic> parseDiagnostics(const QString &output);

signals:
    void jobStarted(const QString &label);
    void outputReceived(const QString &text);
    void diagnosticsChanged(const QList<MalloyWriter::Platform::Diagnostic> &diagnostics);
    void jobFinished(const QString &label, int exitCode, QProcess::ExitStatus status);
    void jobFailedToStart(const QString &label);

private:
    bool startJob(const BuildJob &job);

    ProjectService *m_projectService = nullptr;
    ProcessRunner m_runner;
    BuildJob m_currentJob;
    QList<Diagnostic> m_diagnostics;
};

}

#include "platform/ProcessRunner.hpp"

#include <QDir>

namespace MalloyWriter::Platform {

ProcessRunner::ProcessRunner(QObject *parent)
    : QObject(parent)
{
    m_process.setProcessChannelMode(QProcess::MergedChannels);

    connect(&m_process, &QProcess::started, this, [this]() {
        emit processStarted(m_currentLabel.isEmpty() ? m_process.program() : m_currentLabel);
    });

    connect(&m_process, &QProcess::readyReadStandardOutput, this, [this]() {
        emit outputReceived(QString::fromLocal8Bit(m_process.readAllStandardOutput()));
    });

    connect(&m_process, &QProcess::readyReadStandardError, this, [this]() {
        emit outputReceived(QString::fromLocal8Bit(m_process.readAllStandardError()));
    });

    connect(&m_process, &QProcess::finished, this, [this](int exitCode, QProcess::ExitStatus status) {
        emit processFinished(exitCode, status);
    });
}

bool ProcessRunner::isRunning() const
{
    return m_process.state() != QProcess::NotRunning;
}

bool ProcessRunner::start(const ProcessRequest &request)
{
    if (isRunning() || request.program.trimmed().isEmpty()) {
        return false;
    }

    if (!request.workingDirectory.trimmed().isEmpty()) {
        m_process.setWorkingDirectory(QDir::cleanPath(request.workingDirectory));
    }

    m_currentLabel = request.label;
    emit outputReceived(QString("\n> %1 %2\n")
                            .arg(request.program, request.arguments.join(' ')));
    m_process.start(request.program, request.arguments);
    return m_process.waitForStarted(1000);
}

void ProcessRunner::stop()
{
    if (!isRunning()) {
        return;
    }

    m_process.terminate();
    if (!m_process.waitForFinished(1500)) {
        m_process.kill();
    }
}

}

#pragma once

#include <QObject>
#include <QProcess>
#include <QString>
#include <QStringList>

namespace MalloyWriter::Platform {

struct ProcessRequest {
    QString label;
    QString program;
    QStringList arguments;
    QString workingDirectory;
};

class ProcessRunner : public QObject {
    Q_OBJECT

public:
    explicit ProcessRunner(QObject *parent = nullptr);

    bool isRunning() const;
    bool start(const ProcessRequest &request);
    void stop();

signals:
    void processStarted(const QString &label);
    void outputReceived(const QString &text);
    void processFinished(int exitCode, QProcess::ExitStatus status);

private:
    QProcess m_process;
    QString m_currentLabel;
};

}

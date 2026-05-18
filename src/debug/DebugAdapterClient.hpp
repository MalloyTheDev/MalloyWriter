#pragma once

#include "languages/JsonRpcTransport.hpp"

#include <QObject>
#include <QString>
#include <QStringList>

namespace MalloyWriter::Debug {

struct DebugAdapterDefinition {
    QString id;
    QString displayName;
    QString executable;
    QStringList arguments;
};

class DebugAdapterClient : public QObject {
    Q_OBJECT

public:
    explicit DebugAdapterClient(QObject *parent = nullptr);

    DebugAdapterDefinition adapterDefinition() const;
    bool start(const DebugAdapterDefinition &definition, const QString &workingDirectory);
    void stop();
    bool isRunning() const;
    void initialize();
    void launch(const QString &program, const QString &cwd);
    void setBreakpoint(const QString &sourcePath, int line);
    void continueExecution(int threadId);
    void next(int threadId);
    void disconnectSession();

signals:
    void debugSessionStarted();
    void debugSessionStopped();
    void debugOutputReceived(const QString &text);
    void protocolMessageReceived(const QJsonObject &message);

private:
    void sendRequest(const QString &command, const QJsonObject &arguments = {});

    MalloyWriter::Languages::JsonRpcProcessTransport m_transport;
    int m_nextSequence = 1;
};

}

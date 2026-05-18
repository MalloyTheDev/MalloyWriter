#include "debug/DebugAdapterClient.hpp"

#include <QFileInfo>
#include <QJsonArray>

namespace MalloyWriter::Debug {

DebugAdapterClient::DebugAdapterClient(QObject *parent)
    : QObject(parent)
{
    connect(&m_transport, &MalloyWriter::Languages::JsonRpcProcessTransport::started, this, &DebugAdapterClient::debugSessionStarted);
    connect(&m_transport, &MalloyWriter::Languages::JsonRpcProcessTransport::finished, this, [this](int, QProcess::ExitStatus) {
        emit debugSessionStopped();
    });
    connect(&m_transport, &MalloyWriter::Languages::JsonRpcProcessTransport::stderrReceived, this, &DebugAdapterClient::debugOutputReceived);
    connect(&m_transport, &MalloyWriter::Languages::JsonRpcProcessTransport::messageReceived, this, &DebugAdapterClient::protocolMessageReceived);
}

DebugAdapterDefinition DebugAdapterClient::adapterDefinition() const
{
    return {
        "cppdbg",
        "C++ Debug Adapter",
        "gdb",
        {}
    };
}

bool DebugAdapterClient::start(const DebugAdapterDefinition &definition, const QString &workingDirectory)
{
    return m_transport.start(definition.executable, definition.arguments, workingDirectory);
}

void DebugAdapterClient::stop()
{
    m_transport.stop();
}

bool DebugAdapterClient::isRunning() const
{
    return m_transport.isRunning();
}

void DebugAdapterClient::initialize()
{
    sendRequest("initialize", {
        {"adapterID", "malloywriter"},
        {"clientID", "malloywriter"},
        {"clientName", "MalloyWriter"},
        {"pathFormat", "path"},
        {"linesStartAt1", true},
        {"columnsStartAt1", true},
        {"supportsVariableType", true}
    });
}

void DebugAdapterClient::launch(const QString &program, const QString &cwd)
{
    sendRequest("launch", {
        {"program", QFileInfo(program).absoluteFilePath()},
        {"cwd", cwd},
        {"stopAtEntry", false}
    });
}

void DebugAdapterClient::setBreakpoint(const QString &sourcePath, int line)
{
    sendRequest("setBreakpoints", {
        {"source", QJsonObject{{"path", QFileInfo(sourcePath).absoluteFilePath()}}},
        {"breakpoints", QJsonArray{QJsonObject{{"line", line}}}}
    });
}

void DebugAdapterClient::continueExecution(int threadId)
{
    sendRequest("continue", {{"threadId", threadId}});
}

void DebugAdapterClient::next(int threadId)
{
    sendRequest("next", {{"threadId", threadId}});
}

void DebugAdapterClient::disconnectSession()
{
    sendRequest("disconnect", {{"restart", false}});
}

void DebugAdapterClient::sendRequest(const QString &command, const QJsonObject &arguments)
{
    m_transport.send({
        {"seq", m_nextSequence++},
        {"type", "request"},
        {"command", command},
        {"arguments", arguments}
    });
}

}

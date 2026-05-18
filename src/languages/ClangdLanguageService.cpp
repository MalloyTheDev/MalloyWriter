#include "languages/ClangdLanguageService.hpp"

#include <QFileInfo>
#include <QJsonArray>
#include <QJsonObject>
#include <QUrl>

namespace MalloyWriter::Languages {

ClangdLanguageService::ClangdLanguageService(QObject *parent)
    : QObject(parent)
{
    connect(&m_transport, &JsonRpcProcessTransport::started, this, [this]() {
        emit statusChanged("clangd started");
    });
    connect(&m_transport, &JsonRpcProcessTransport::finished, this, [this](int, QProcess::ExitStatus) {
        emit statusChanged("clangd stopped");
    });
    connect(&m_transport, &JsonRpcProcessTransport::stderrReceived, this, &ClangdLanguageService::stderrReceived);
    connect(&m_transport, &JsonRpcProcessTransport::protocolError, this, &ClangdLanguageService::statusChanged);
    connect(&m_transport, &JsonRpcProcessTransport::messageReceived, this, [this](const QJsonObject &message) {
        if (message.value("method").toString() == "textDocument/publishDiagnostics") {
            const QJsonObject params = message.value("params").toObject();
            emit diagnosticsReceived(params.value("uri").toString(), params.value("diagnostics").toArray());
        }
    });
}

bool ClangdLanguageService::start(const MalloyWriter::Platform::ToolchainKit &kit, const QString &workspaceRoot)
{
    if (!m_transport.start(kit.clangdProgram, {"--background-index", "--clang-tidy"}, workspaceRoot)) {
        emit statusChanged("Failed to start clangd");
        return false;
    }
    sendInitialize(workspaceRoot);
    return true;
}

void ClangdLanguageService::stop()
{
    m_transport.stop();
}

bool ClangdLanguageService::isRunning() const
{
    return m_transport.isRunning();
}

void ClangdLanguageService::didOpen(const QString &filePath, const QString &languageId, const QString &text)
{
    m_transport.send({
        {"jsonrpc", "2.0"},
        {"method", "textDocument/didOpen"},
        {"params", QJsonObject{
            {"textDocument", QJsonObject{
                {"uri", fileUri(filePath)},
                {"languageId", languageId},
                {"version", 1},
                {"text", text}
            }}
        }}
    });
}

void ClangdLanguageService::didChange(const QString &filePath, const QString &text, int version)
{
    m_transport.send({
        {"jsonrpc", "2.0"},
        {"method", "textDocument/didChange"},
        {"params", QJsonObject{
            {"textDocument", QJsonObject{{"uri", fileUri(filePath)}, {"version", version}}},
            {"contentChanges", QJsonArray{QJsonObject{{"text", text}}}}
        }}
    });
}

void ClangdLanguageService::didSave(const QString &filePath)
{
    m_transport.send({
        {"jsonrpc", "2.0"},
        {"method", "textDocument/didSave"},
        {"params", QJsonObject{{"textDocument", QJsonObject{{"uri", fileUri(filePath)}}}}}
    });
}

QString ClangdLanguageService::fileUri(const QString &path) const
{
    return QUrl::fromLocalFile(QFileInfo(path).absoluteFilePath()).toString();
}

void ClangdLanguageService::sendInitialize(const QString &workspaceRoot)
{
    const QString rootUri = QUrl::fromLocalFile(QFileInfo(workspaceRoot).absoluteFilePath()).toString();
    m_transport.send({
        {"jsonrpc", "2.0"},
        {"id", m_nextId++},
        {"method", "initialize"},
        {"params", QJsonObject{
            {"processId", QJsonValue::Null},
            {"rootUri", rootUri},
            {"capabilities", QJsonObject{
                {"textDocument", QJsonObject{
                    {"synchronization", QJsonObject{{"didSave", true}}},
                    {"completion", QJsonObject{}},
                    {"hover", QJsonObject{}},
                    {"definition", QJsonObject{}},
                    {"references", QJsonObject{}},
                    {"documentSymbol", QJsonObject{}},
                    {"rename", QJsonObject{}}
                }},
                {"workspace", QJsonObject{{"configuration", true}}}
            }}
        }}
    });
}

}

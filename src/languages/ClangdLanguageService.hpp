#pragma once

#include "languages/JsonRpcTransport.hpp"
#include "platform/ToolchainKit.hpp"

#include <QObject>

namespace MalloyWriter::Languages {

class ClangdLanguageService : public QObject {
    Q_OBJECT

public:
    explicit ClangdLanguageService(QObject *parent = nullptr);

    bool start(const MalloyWriter::Platform::ToolchainKit &kit, const QString &workspaceRoot);
    void stop();
    bool isRunning() const;
    void didOpen(const QString &filePath, const QString &languageId, const QString &text);
    void didChange(const QString &filePath, const QString &text, int version);
    void didSave(const QString &filePath);

signals:
    void statusChanged(const QString &status);
    void diagnosticsReceived(const QString &uri, const QJsonArray &diagnostics);
    void stderrReceived(const QString &text);

private:
    QString fileUri(const QString &path) const;
    void sendInitialize(const QString &workspaceRoot);

    JsonRpcProcessTransport m_transport;
    int m_nextId = 1;
};

}

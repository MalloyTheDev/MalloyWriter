#include "languages/JsonRpcTransport.hpp"

#include <QDir>
#include <QJsonDocument>

namespace MalloyWriter::Languages {

QByteArray JsonRpcFraming::encode(const QJsonObject &message)
{
    const QByteArray payload = QJsonDocument(message).toJson(QJsonDocument::Compact);
    return QByteArray("Content-Length: ") + QByteArray::number(payload.size()) + "\r\n\r\n" + payload;
}

QList<QJsonObject> JsonRpcFraming::extractMessages(QByteArray &buffer, QString *errorMessage)
{
    QList<QJsonObject> messages;
    while (true) {
        const qsizetype headerEnd = buffer.indexOf("\r\n\r\n");
        if (headerEnd < 0) {
            break;
        }

        const QByteArray header = buffer.left(headerEnd);
        int contentLength = -1;
        for (const QByteArray &line : header.split('\n')) {
            const QByteArray trimmed = line.trimmed();
            if (trimmed.toLower().startsWith("content-length:")) {
                contentLength = trimmed.mid(strlen("content-length:")).trimmed().toInt();
            }
        }

        if (contentLength < 0) {
            if (errorMessage) {
                *errorMessage = "JSON-RPC message is missing Content-Length.";
            }
            buffer.clear();
            break;
        }

        const qsizetype payloadStart = headerEnd + 4;
        if (buffer.size() < payloadStart + contentLength) {
            break;
        }

        const QByteArray payload = buffer.mid(payloadStart, contentLength);
        buffer.remove(0, payloadStart + contentLength);

        QJsonParseError parseError;
        const QJsonDocument document = QJsonDocument::fromJson(payload, &parseError);
        if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
            if (errorMessage) {
                *errorMessage = QString("Invalid JSON-RPC payload: %1").arg(parseError.errorString());
            }
            continue;
        }
        messages << document.object();
    }
    return messages;
}

JsonRpcProcessTransport::JsonRpcProcessTransport(QObject *parent)
    : QObject(parent)
{
    m_process.setProcessChannelMode(QProcess::SeparateChannels);

    connect(&m_process, &QProcess::started, this, &JsonRpcProcessTransport::started);
    connect(&m_process, &QProcess::finished, this, &JsonRpcProcessTransport::finished);
    connect(&m_process, &QProcess::readyReadStandardError, this, [this]() {
        emit stderrReceived(QString::fromLocal8Bit(m_process.readAllStandardError()));
    });
    connect(&m_process, &QProcess::readyReadStandardOutput, this, [this]() {
        m_buffer.append(m_process.readAllStandardOutput());
        QString error;
        for (const QJsonObject &message : JsonRpcFraming::extractMessages(m_buffer, &error)) {
            emit messageReceived(message);
        }
        if (!error.isEmpty()) {
            emit protocolError(error);
        }
    });
}

bool JsonRpcProcessTransport::start(const QString &program, const QStringList &arguments, const QString &workingDirectory)
{
    if (isRunning() || program.trimmed().isEmpty()) {
        return false;
    }
    if (!workingDirectory.trimmed().isEmpty()) {
        m_process.setWorkingDirectory(QDir::cleanPath(workingDirectory));
    }
    m_process.start(program, arguments);
    return m_process.waitForStarted(1000);
}

bool JsonRpcProcessTransport::isRunning() const
{
    return m_process.state() != QProcess::NotRunning;
}

void JsonRpcProcessTransport::stop()
{
    if (!isRunning()) {
        return;
    }
    m_process.terminate();
    if (!m_process.waitForFinished(1500)) {
        m_process.kill();
    }
}

void JsonRpcProcessTransport::send(const QJsonObject &message)
{
    if (!isRunning()) {
        return;
    }
    m_process.write(JsonRpcFraming::encode(message));
}

}

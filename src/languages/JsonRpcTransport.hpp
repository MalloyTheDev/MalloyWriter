#pragma once

#include <QByteArray>
#include <QJsonObject>
#include <QObject>
#include <QProcess>
#include <QString>

namespace MalloyWriter::Languages {

class JsonRpcFraming {
public:
    static QByteArray encode(const QJsonObject &message);
    static QList<QJsonObject> extractMessages(QByteArray &buffer, QString *errorMessage = nullptr);
};

class JsonRpcProcessTransport : public QObject {
    Q_OBJECT

public:
    explicit JsonRpcProcessTransport(QObject *parent = nullptr);

    bool start(const QString &program, const QStringList &arguments, const QString &workingDirectory = {});
    bool isRunning() const;
    void stop();
    void send(const QJsonObject &message);

signals:
    void messageReceived(const QJsonObject &message);
    void stderrReceived(const QString &text);
    void started();
    void finished(int exitCode, QProcess::ExitStatus status);
    void protocolError(const QString &message);

private:
    QProcess m_process;
    QByteArray m_buffer;
};

}

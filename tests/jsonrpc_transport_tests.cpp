#include "languages/JsonRpcTransport.hpp"

#include <QJsonObject>
#include <QtTest/QtTest>

using MalloyWriter::Languages::JsonRpcFraming;

class JsonRpcTransportTests : public QObject {
    Q_OBJECT

private slots:
    void encodesAndExtractsMessage()
    {
        QJsonObject message{{"jsonrpc", "2.0"}, {"id", 1}, {"method", "initialize"}};
        QByteArray buffer = JsonRpcFraming::encode(message);
        QString error;
        const auto messages = JsonRpcFraming::extractMessages(buffer, &error);

        QVERIFY(error.isEmpty());
        QCOMPARE(messages.size(), 1);
        QCOMPARE(messages[0].value("method").toString(), QString("initialize"));
        QVERIFY(buffer.isEmpty());
    }

    void waitsForPartialPayload()
    {
        QByteArray encoded = JsonRpcFraming::encode({{"jsonrpc", "2.0"}, {"method", "test"}});
        QByteArray partial = encoded.left(encoded.size() - 2);
        QString error;
        const auto messages = JsonRpcFraming::extractMessages(partial, &error);

        QVERIFY(error.isEmpty());
        QVERIFY(messages.isEmpty());
        QVERIFY(!partial.isEmpty());
    }
};

QTEST_MAIN(JsonRpcTransportTests)

#include "jsonrpc_transport_tests.moc"

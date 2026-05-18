#include "platform/DocumentService.hpp"

#include <QFile>
#include <QTemporaryDir>
#include <QtTest/QtTest>

using MalloyWriter::Platform::DocumentService;

class DocumentServiceTests : public QObject {
    Q_OBJECT

private slots:
    void reusesOpenDocument()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString path = dir.filePath("main.cpp");
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
        file.write("int main() {}\n");
        file.close();

        DocumentService service;
        auto *first = service.openDocument(path);
        auto *second = service.openDocument(path);
        QVERIFY(first);
        QCOMPARE(first, second);
        QCOMPARE(service.openDocuments().size(), 1);
    }

    void savesOpenDocument()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString path = dir.filePath("notes.txt");
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
        file.write("before\n");
        file.close();

        DocumentService service;
        auto *document = service.openDocument(path);
        QVERIFY(document);
        document->setText("after\n");

        QString error;
        QVERIFY(service.saveDocument(document, &error));
        QVERIFY(error.isEmpty());

        QFile saved(path);
        QVERIFY(saved.open(QIODevice::ReadOnly | QIODevice::Text));
        QCOMPARE(QString::fromUtf8(saved.readAll()), QString("after\n"));
    }
};

QTEST_MAIN(DocumentServiceTests)

#include "document_service_tests.moc"

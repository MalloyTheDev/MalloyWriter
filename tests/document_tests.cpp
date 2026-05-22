#include "editor/Document.hpp"

#include <QFile>
#include <QTemporaryDir>
#include <QtTest/QtTest>

using MalloyWriter::Editor::Document;

class DocumentTests : public QObject {
    Q_OBJECT

private slots:
    void loadsTextAndStartsClean()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());

        const QString path = dir.filePath("main.cpp");
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
        file.write("int main() { return 0; }\n");
        file.close();

        Document document;
        QVERIFY(document.load(path));
        QCOMPARE(document.text(), QString("int main() { return 0; }\n"));
        QVERIFY(!document.isDirty());
        QCOMPARE(document.fileName(), QString("main.cpp"));
    }

    void loadNormalizesCrlfAndStaysClean()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());

        const QString path = dir.filePath("crlf.cpp");
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly)); // raw write keeps CRLF
        file.write("int a;\r\nint b;\r\n");
        file.close();

        Document document;
        QVERIFY(document.load(path));
        QVERIFY(!document.isDirty());
        QVERIFY(!document.text().contains(QLatin1Char('\r')));
        QCOMPARE(document.text(), QString("int a;\nint b;\n"));
    }

    void marksDirtyAndSaves()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());

        const QString path = dir.filePath("notes.txt");
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
        file.write("before");
        file.close();

        Document document;
        QVERIFY(document.load(path));
        document.setText("after");
        QVERIFY(document.isDirty());
        QVERIFY(document.save());
        QVERIFY(!document.isDirty());

        QFile saved(path);
        QVERIFY(saved.open(QIODevice::ReadOnly | QIODevice::Text));
        QCOMPARE(QString::fromUtf8(saved.readAll()), QString("after"));
    }
};

QTEST_MAIN(DocumentTests)

#include "document_tests.moc"

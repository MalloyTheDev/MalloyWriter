#include "platform/CMakeFileApi.hpp"

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>
#include <QtTest/QtTest>

using MalloyWriter::Platform::CMakeFileApi;

class CMakeFileApiTests : public QObject {
    Q_OBJECT

private slots:
    void writesQueryFile()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());

        QString error;
        QVERIFY(CMakeFileApi::writeQuery(dir.path(), &error));
        QVERIFY(error.isEmpty());
        QVERIFY(QFile::exists(QDir(dir.path()).filePath(".cmake/api/v1/query/client-malloywriter/query.json")));
    }

    void readsTargetModel()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString replyDir = QDir(dir.path()).filePath(".cmake/api/v1/reply");
        QVERIFY(QDir().mkpath(replyDir));

        writeJson(QDir(replyDir).filePath("index-1.json"), {
            {"cmake", QJsonObject{
                {"version", QJsonObject{{"string", "4.2.3"}}},
                {"generator", QJsonObject{{"name", "Ninja"}}}
            }},
            {"objects", QJsonArray{QJsonObject{{"kind", "codemodel"}, {"jsonFile", "codemodel-v2.json"}}}}
        });

        writeJson(QDir(replyDir).filePath("codemodel-v2.json"), {
            {"configurations", QJsonArray{QJsonObject{{"projects", QJsonArray{QJsonObject{{"targets", QJsonArray{QJsonObject{{"id", "app::@root"}, {"name", "app"}, {"jsonFile", "target-app.json"}}}}}}}}}}
        });

        writeJson(QDir(replyDir).filePath("target-app.json"), {
            {"id", "app::@root"},
            {"name", "app"},
            {"type", "EXECUTABLE"},
            {"sourceDirectory", "E:/Source"},
            {"buildDirectory", "E:/Build"},
            {"artifacts", QJsonArray{QJsonObject{{"path", "app.exe"}}}}
        });

        QString error;
        const auto model = CMakeFileApi::readModel("E:/Source", dir.path(), &error);
        QVERIFY(error.isEmpty());
        QCOMPARE(model.generator, QString("Ninja"));
        QCOMPARE(model.cmakeVersion, QString("4.2.3"));
        QCOMPARE(model.targets.size(), 1);
        QCOMPARE(model.targets[0].name, QString("app"));
        QCOMPARE(model.targets[0].type, QString("EXECUTABLE"));
        QCOMPARE(model.targets[0].artifacts.value(0), QString("app.exe"));
    }

private:
    static void writeJson(const QString &path, const QJsonObject &object)
    {
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate));
        file.write(QJsonDocument(object).toJson(QJsonDocument::Indented));
    }
};

QTEST_MAIN(CMakeFileApiTests)

#include "cmake_file_api_tests.moc"

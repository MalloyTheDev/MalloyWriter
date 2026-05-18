#include "platform/CMakeFileApi.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <algorithm>

namespace MalloyWriter::Platform {
namespace {

QJsonObject readJsonObject(const QString &path, QString *errorMessage)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (errorMessage) {
            *errorMessage = QString("Could not open %1").arg(path);
        }
        return {};
    }

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        if (errorMessage) {
            *errorMessage = QString("Invalid CMake File API JSON in %1: %2").arg(path, parseError.errorString());
        }
        return {};
    }

    return document.object();
}

QString latestIndexPath(const QString &buildDirectory)
{
    const QDir replyDir(QDir(buildDirectory).filePath(".cmake/api/v1/reply"));
    const QFileInfoList entries = replyDir.entryInfoList({"index-*.json"}, QDir::Files, QDir::Name);
    if (entries.isEmpty()) {
        return {};
    }
    return entries.last().absoluteFilePath();
}

QString replyPathFor(const QString &buildDirectory, const QString &jsonFile)
{
    return QDir(buildDirectory).filePath(QString(".cmake/api/v1/reply/%1").arg(jsonFile));
}

CMakeTarget readTarget(const QString &buildDirectory, const QJsonObject &targetReference)
{
    QString error;
    const QJsonObject targetObject = readJsonObject(replyPathFor(buildDirectory, targetReference.value("jsonFile").toString()), &error);

    CMakeTarget target;
    target.id = targetObject.value("id").toString(targetReference.value("id").toString());
    target.name = targetObject.value("name").toString(targetReference.value("name").toString());
    target.type = targetObject.value("type").toString();
    target.sourceDirectory = targetObject.value("sourceDirectory").toString();
    target.buildDirectory = targetObject.value("buildDirectory").toString();

    const QJsonArray artifacts = targetObject.value("artifacts").toArray();
    for (const QJsonValue &artifactValue : artifacts) {
        const QJsonObject artifact = artifactValue.toObject();
        const QString path = artifact.value("path").toString();
        if (!path.isEmpty()) {
            target.artifacts << path;
        }
    }

    return target;
}

}

bool CMakeFileApi::writeQuery(const QString &buildDirectory, QString *errorMessage)
{
    const QString queryDirPath = QDir(buildDirectory).filePath(".cmake/api/v1/query/client-malloywriter");
    QDir queryDir;
    if (!queryDir.mkpath(queryDirPath)) {
        if (errorMessage) {
            *errorMessage = QString("Could not create %1").arg(queryDirPath);
        }
        return false;
    }

    QFile queryFile(QDir(queryDirPath).filePath("query.json"));
    if (!queryFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        if (errorMessage) {
            *errorMessage = QString("Could not write %1").arg(queryFile.fileName());
        }
        return false;
    }

    const QJsonObject query{
        {"requests", QJsonArray{
            QJsonObject{{"kind", "codemodel"}, {"version", QJsonObject{{"major", 2}}}},
            QJsonObject{{"kind", "cache"}, {"version", QJsonObject{{"major", 2}}}},
            QJsonObject{{"kind", "toolchains"}, {"version", QJsonObject{{"major", 1}}}}
        }}
    };
    queryFile.write(QJsonDocument(query).toJson(QJsonDocument::Indented));
    return true;
}

bool CMakeFileApi::hasReply(const QString &buildDirectory)
{
    return !latestIndexPath(buildDirectory).isEmpty();
}

CMakeProjectModel CMakeFileApi::readModel(const QString &sourceDirectory, const QString &buildDirectory, QString *errorMessage)
{
    CMakeProjectModel model;
    model.sourceDirectory = QDir::cleanPath(QFileInfo(sourceDirectory).absoluteFilePath());
    model.buildDirectory = QDir::cleanPath(QFileInfo(buildDirectory).absoluteFilePath());

    const QString indexPath = latestIndexPath(buildDirectory);
    if (indexPath.isEmpty()) {
        if (errorMessage) {
            *errorMessage = "CMake File API reply is not available. Run configure first.";
        }
        return model;
    }

    const QJsonObject index = readJsonObject(indexPath, errorMessage);
    if (index.isEmpty()) {
        return model;
    }

    const QJsonObject cmake = index.value("cmake").toObject();
    model.cmakeVersion = cmake.value("version").toObject().value("string").toString();
    model.generator = cmake.value("generator").toObject().value("name").toString();

    QString codemodelJsonFile;
    const QJsonArray objects = index.value("objects").toArray();
    for (const QJsonValue &objectValue : objects) {
        const QJsonObject object = objectValue.toObject();
        if (object.value("kind").toString() == "codemodel") {
            codemodelJsonFile = object.value("jsonFile").toString();
            break;
        }
    }

    if (codemodelJsonFile.isEmpty()) {
        if (errorMessage) {
            *errorMessage = "CMake File API codemodel reply was not found.";
        }
        return model;
    }

    const QJsonObject codemodel = readJsonObject(replyPathFor(buildDirectory, codemodelJsonFile), errorMessage);
    const QJsonArray configurations = codemodel.value("configurations").toArray();
    for (const QJsonValue &configurationValue : configurations) {
        const QJsonObject configuration = configurationValue.toObject();
        const QJsonArray projects = configuration.value("projects").toArray();
        for (const QJsonValue &projectValue : projects) {
            const QJsonArray targets = projectValue.toObject().value("targets").toArray();
            for (const QJsonValue &targetValue : targets) {
                CMakeTarget target = readTarget(buildDirectory, targetValue.toObject());
                if (!target.name.isEmpty()) {
                    model.targets << target;
                }
            }
        }
    }

    std::sort(model.targets.begin(), model.targets.end(), [](const CMakeTarget &left, const CMakeTarget &right) {
        return left.name.localeAwareCompare(right.name) < 0;
    });

    return model;
}

}

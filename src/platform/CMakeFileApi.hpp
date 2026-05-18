#pragma once

#include <QList>
#include <QString>

namespace MalloyWriter::Platform {

struct CMakeTarget {
    QString id;
    QString name;
    QString type;
    QString sourceDirectory;
    QString buildDirectory;
    QStringList artifacts;
};

struct CMakeProjectModel {
    QString sourceDirectory;
    QString buildDirectory;
    QString generator;
    QString cmakeVersion;
    QList<CMakeTarget> targets;
};

class CMakeFileApi {
public:
    static bool writeQuery(const QString &buildDirectory, QString *errorMessage = nullptr);
    static bool hasReply(const QString &buildDirectory);
    static CMakeProjectModel readModel(const QString &sourceDirectory, const QString &buildDirectory, QString *errorMessage = nullptr);
};

}

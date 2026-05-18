#pragma once

#include "platform/BuildSystem.hpp"
#include "platform/CMakeFileApi.hpp"
#include "platform/ToolchainKit.hpp"

#include <QObject>
#include <QString>

namespace MalloyWriter::Platform {

class ProjectService : public QObject {
    Q_OBJECT

public:
    explicit ProjectService(QObject *parent = nullptr);

    bool openWorkspace(const QString &path);
    void clear();

    bool hasWorkspace() const;
    QString workspaceRoot() const;
    QString buildDirectory() const;
    ToolchainKit activeKit() const;
    BuildSettings buildSettings() const;
    CMakeProjectModel cmakeModel() const;
    QStringList executableTargets() const;

    bool prepareCMakeQuery(QString *errorMessage = nullptr);
    bool refreshCMakeModel(QString *errorMessage = nullptr);
    void setActiveKit(const ToolchainKit &kit);

signals:
    void workspaceChanged(const QString &rootPath);
    void cmakeModelChanged();
    void kitChanged();

private:
    QString m_workspaceRoot;
    ToolchainKit m_activeKit;
    CMakeProjectModel m_cmakeModel;
};

}

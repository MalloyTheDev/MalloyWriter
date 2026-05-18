#pragma once

#include "platform/BuildSystem.hpp"

#include <QObject>
#include <QString>
#include <QStringList>

namespace MalloyWriter::Platform {

class SettingsService : public QObject {
    Q_OBJECT

public:
    explicit SettingsService(QObject *parent = nullptr);

    BuildSettings buildSettings() const;
    void setBuildSettings(const BuildSettings &settings);

    QStringList recentWorkspaces() const;
    void addRecentWorkspace(const QString &path);

    QStringList recentFiles() const;
    void addRecentFile(const QString &path);

    QString lastRunExecutable() const;
    void setLastRunExecutable(const QString &path);

signals:
    void recentWorkspacesChanged();
    void recentFilesChanged();

private:
    QStringList listValue(const QString &key) const;
    void addRecentPath(const QString &key, const QString &path, int maxItems);
};

}

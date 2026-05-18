#pragma once

#include <QObject>
#include <QString>

namespace MalloyWriter::Platform {

class WorkspaceModel : public QObject {
    Q_OBJECT

public:
    explicit WorkspaceModel(QObject *parent = nullptr);

    bool openFolder(const QString &path);
    void clear();
    bool hasWorkspace() const;
    QString rootPath() const;

signals:
    void workspaceChanged(const QString &rootPath);

private:
    QString m_rootPath;
};

}

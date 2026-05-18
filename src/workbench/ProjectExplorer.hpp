#pragma once

#include <QWidget>

class QFileSystemModel;
class QTreeView;

namespace MalloyWriter::Workbench {

class ProjectExplorer : public QWidget {
    Q_OBJECT

public:
    explicit ProjectExplorer(QWidget *parent = nullptr);

    void setRootPath(const QString &path);
    QString rootPath() const;

signals:
    void fileActivated(const QString &path);

private:
    QFileSystemModel *m_model = nullptr;
    QTreeView *m_tree = nullptr;
    QString m_rootPath;
};

}

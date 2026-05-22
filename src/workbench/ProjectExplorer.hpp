#pragma once

#include <QFileIconProvider>
#include <QWidget>

#include <memory>

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
    std::unique_ptr<QFileIconProvider> m_iconProvider;
    QString m_rootPath;
};

}

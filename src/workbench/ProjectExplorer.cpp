#include "workbench/ProjectExplorer.hpp"

#include <QDir>
#include <QFileInfo>
#include <QFileSystemModel>
#include <QTreeView>
#include <QVBoxLayout>

namespace MalloyWriter::Workbench {

ProjectExplorer::ProjectExplorer(QWidget *parent)
    : QWidget(parent)
    , m_model(new QFileSystemModel(this))
    , m_tree(new QTreeView(this))
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_tree);

    m_model->setFilter(QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot);
    m_tree->setModel(m_model);
    m_tree->setHeaderHidden(true);
    m_tree->setAnimated(false);
    m_tree->setUniformRowHeights(true);

    for (int column = 1; column < m_model->columnCount(); ++column) {
        m_tree->hideColumn(column);
    }

    connect(m_tree, &QTreeView::doubleClicked, this, [this](const QModelIndex &index) {
        const QString path = m_model->filePath(index);
        if (QFileInfo(path).isFile()) {
            emit fileActivated(path);
        }
    });
}

void ProjectExplorer::setRootPath(const QString &path)
{
    m_rootPath = path;
    const QModelIndex rootIndex = m_model->setRootPath(path);
    m_tree->setRootIndex(rootIndex);
}

QString ProjectExplorer::rootPath() const
{
    return m_rootPath;
}

}

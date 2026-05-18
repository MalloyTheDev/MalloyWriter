#include "platform/WorkspaceModel.hpp"

#include <QDir>
#include <QFileInfo>

namespace MalloyWriter::Platform {

WorkspaceModel::WorkspaceModel(QObject *parent)
    : QObject(parent)
{
}

bool WorkspaceModel::openFolder(const QString &path)
{
    const QFileInfo info(path);
    if (!info.exists() || !info.isDir()) {
        return false;
    }

    const QString normalized = QDir::cleanPath(info.absoluteFilePath());
    if (m_rootPath == normalized) {
        return true;
    }

    m_rootPath = normalized;
    emit workspaceChanged(m_rootPath);
    return true;
}

void WorkspaceModel::clear()
{
    if (m_rootPath.isEmpty()) {
        return;
    }

    m_rootPath.clear();
    emit workspaceChanged(m_rootPath);
}

bool WorkspaceModel::hasWorkspace() const
{
    return !m_rootPath.isEmpty();
}

QString WorkspaceModel::rootPath() const
{
    return m_rootPath;
}

}

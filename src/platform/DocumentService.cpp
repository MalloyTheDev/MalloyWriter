#include "platform/DocumentService.hpp"

#include <QDir>
#include <QFileInfo>

namespace MalloyWriter::Platform {

DocumentService::DocumentService(QObject *parent)
    : QObject(parent)
{
}

MalloyWriter::Editor::Document *DocumentService::openDocument(const QString &path)
{
    const QString key = normalizedPath(path);
    if (auto *existing = m_documentsByPath.value(key, nullptr)) {
        return existing;
    }

    auto *document = new MalloyWriter::Editor::Document(this);
    if (!document->load(key)) {
        document->deleteLater();
        return nullptr;
    }

    m_documentsByPath.insert(key, document);
    emit documentOpened(document);
    return document;
}

MalloyWriter::Editor::Document *DocumentService::documentForPath(const QString &path) const
{
    return m_documentsByPath.value(normalizedPath(path), nullptr);
}

QList<MalloyWriter::Editor::Document *> DocumentService::openDocuments() const
{
    return m_documentsByPath.values();
}

bool DocumentService::saveDocument(MalloyWriter::Editor::Document *document, QString *errorMessage)
{
    if (!document) {
        if (errorMessage) {
            *errorMessage = "No document is selected.";
        }
        return false;
    }

    if (document->hasExternalModification()) {
        emit externalModificationDetected(document);
        if (errorMessage) {
            *errorMessage = QString("%1 changed on disk. Reload or save as a new file before overwriting.").arg(document->path());
        }
        return false;
    }

    if (!document->save()) {
        if (errorMessage) {
            *errorMessage = QString("Could not save %1").arg(document->path());
        }
        return false;
    }

    emit documentSaved(document);
    return true;
}

bool DocumentService::saveAll(QStringList *failedPaths)
{
    bool allSaved = true;
    for (auto *document : openDocuments()) {
        QString error;
        if (!saveDocument(document, &error)) {
            allSaved = false;
            if (failedPaths) {
                failedPaths->append(document->path());
            }
        }
    }
    return allSaved;
}

QString DocumentService::normalizedPath(const QString &path) const
{
    return QDir::cleanPath(QFileInfo(path).absoluteFilePath());
}

}

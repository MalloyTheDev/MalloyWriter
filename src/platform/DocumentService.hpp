#pragma once

#include "editor/Document.hpp"

#include <QHash>
#include <QObject>

namespace MalloyWriter::Platform {

class DocumentService : public QObject {
    Q_OBJECT

public:
    explicit DocumentService(QObject *parent = nullptr);

    MalloyWriter::Editor::Document *openDocument(const QString &path);
    MalloyWriter::Editor::Document *documentForPath(const QString &path) const;
    QList<MalloyWriter::Editor::Document *> openDocuments() const;
    bool saveDocument(MalloyWriter::Editor::Document *document, QString *errorMessage = nullptr);
    bool saveAll(QStringList *failedPaths = nullptr);

signals:
    void documentOpened(MalloyWriter::Editor::Document *document);
    void documentSaved(MalloyWriter::Editor::Document *document);
    void externalModificationDetected(MalloyWriter::Editor::Document *document);

private:
    QString normalizedPath(const QString &path) const;

    QHash<QString, MalloyWriter::Editor::Document *> m_documentsByPath;
};

}

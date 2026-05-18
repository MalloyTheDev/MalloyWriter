#pragma once

#include "editor/Document.hpp"
#include "platform/Diagnostic.hpp"

#include <QHash>
#include <QTabWidget>

namespace MalloyWriter::Editor {

class EditorWidget;

class EditorArea : public QTabWidget {
    Q_OBJECT

public:
    explicit EditorArea(QWidget *parent = nullptr);

    Document *currentDocument() const;
    bool openFile(const QString &path);
    bool openDocument(Document *document);
    bool saveCurrent();
    bool saveAll();
    void setDiagnostics(const QList<MalloyWriter::Platform::Diagnostic> &diagnostics);
    bool findInCurrent(const QString &text, bool caseSensitive = false);
    bool replaceInCurrent(const QString &findText, const QString &replaceText, bool caseSensitive = false);
    void goToLineInCurrent(int line);

signals:
    void fileOpened(const QString &path);
    void currentDocumentChanged(Document *document);

private:
    void closeEditorTab(int index);
    void updateTabTitle(Document *document);
    EditorWidget *widgetForDocument(Document *document) const;
    EditorWidget *currentEditor() const;

    QHash<QString, Document *> m_openDocumentsByPath;
};

}

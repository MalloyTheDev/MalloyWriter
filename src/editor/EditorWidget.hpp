#pragma once

#include "editor/Document.hpp"
#include "platform/Diagnostic.hpp"

#include <QWidget>

namespace MalloyWriter::Editor {

class CodeEditor;

class EditorWidget : public QWidget {
    Q_OBJECT

public:
    explicit EditorWidget(Document *document, QWidget *parent = nullptr);

    Document *documentModel() const;
    CodeEditor *textEdit() const;
    void setDiagnostics(const QList<MalloyWriter::Platform::Diagnostic> &diagnostics);
    bool findNext(const QString &text, bool caseSensitive = false);
    bool replaceNext(const QString &findText, const QString &replaceText, bool caseSensitive = false);
    void goToLine(int line);

private:
    void reloadFromDocument();

    Document *m_document = nullptr;
    CodeEditor *m_textEdit = nullptr;
    bool m_applyingDocumentText = false;
};

}

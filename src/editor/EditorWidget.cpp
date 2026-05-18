#include "editor/EditorWidget.hpp"

#include "editor/CodeEditor.hpp"

#include <QFontDatabase>
#include <QVBoxLayout>

namespace MalloyWriter::Editor {

EditorWidget::EditorWidget(Document *document, QWidget *parent)
    : QWidget(parent)
    , m_document(document)
    , m_textEdit(new CodeEditor(this))
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_textEdit);

    QFont editorFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    editorFont.setPointSize(10);
    m_textEdit->setFont(editorFont);
    m_textEdit->setLineWrapMode(QPlainTextEdit::NoWrap);
    m_textEdit->setTabStopDistance(m_textEdit->fontMetrics().horizontalAdvance(' ') * 4);

    reloadFromDocument();

    connect(m_textEdit, &QPlainTextEdit::textChanged, this, [this]() {
        if (!m_document || m_applyingDocumentText) {
            return;
        }
        m_document->setText(m_textEdit->toPlainText());
    });

    connect(m_document, &Document::textChanged, this, [this]() {
        if (!m_document || m_textEdit->toPlainText() == m_document->text()) {
            return;
        }
        reloadFromDocument();
    });
}

Document *EditorWidget::documentModel() const
{
    return m_document;
}

CodeEditor *EditorWidget::textEdit() const
{
    return m_textEdit;
}

void EditorWidget::setDiagnostics(const QList<MalloyWriter::Platform::Diagnostic> &diagnostics)
{
    m_textEdit->setDiagnostics(diagnostics);
}

bool EditorWidget::findNext(const QString &text, bool caseSensitive)
{
    return m_textEdit->findNext(text, caseSensitive);
}

bool EditorWidget::replaceNext(const QString &findText, const QString &replaceText, bool caseSensitive)
{
    return m_textEdit->replaceNext(findText, replaceText, caseSensitive);
}

void EditorWidget::goToLine(int line)
{
    m_textEdit->goToLine(line);
}

void EditorWidget::reloadFromDocument()
{
    if (!m_document) {
        return;
    }

    m_applyingDocumentText = true;
    m_textEdit->setPlainText(m_document->text());
    m_applyingDocumentText = false;
}

}

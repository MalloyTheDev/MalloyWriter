#include "editor/EditorWidget.hpp"

#include "editor/Breadcrumbs.hpp"
#include "editor/CodeEditor.hpp"
#include "editor/CppSyntaxHighlighter.hpp"
#include "editor/Minimap.hpp"

#include <QFileInfo>
#include <QFont>
#include <QHBoxLayout>
#include <QVBoxLayout>

namespace MalloyWriter::Editor {

namespace {

bool isCppFile(const QString &fileName)
{
    static const QStringList suffixes = {
        "cpp", "cc", "cxx", "c", "hpp", "hxx", "h", "inl", "ipp", "tpp",
    };
    return suffixes.contains(QFileInfo(fileName).suffix().toLower());
}

} // namespace

EditorWidget::EditorWidget(Document *document, QWidget *parent)
    : QWidget(parent)
    , m_document(document)
    , m_textEdit(new CodeEditor(this))
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    m_breadcrumbs = new Breadcrumbs(this);
    root->addWidget(m_breadcrumbs);

    auto *editorRow = new QWidget(this);
    auto *rowLayout = new QHBoxLayout(editorRow);
    rowLayout->setContentsMargins(0, 0, 0, 0);
    rowLayout->setSpacing(0);
    rowLayout->addWidget(m_textEdit, 1);
    m_minimap = new Minimap(m_textEdit, editorRow);
    rowLayout->addWidget(m_minimap);
    root->addWidget(editorRow, 1);

    QFont editorFont;
    editorFont.setFamilies({"Geist Mono", "Cascadia Mono", "JetBrains Mono", "Consolas"});
    editorFont.setStyleHint(QFont::Monospace);
    editorFont.setFixedPitch(true);
    editorFont.setPointSize(10);
    m_textEdit->setFont(editorFont);
    m_textEdit->setLineWrapMode(QPlainTextEdit::NoWrap);
    m_textEdit->setTabStopDistance(m_textEdit->fontMetrics().horizontalAdvance(' ') * 4);

    // C/C++ files get syntax highlighting (parented to the document).
    if (m_document && isCppFile(m_document->fileName())) {
        new CppSyntaxHighlighter(m_textEdit->document());
    }

    if (m_document) {
        m_breadcrumbs->setFilePath(m_document->path());
        connect(m_document, &Document::pathChanged, this, [this](const QString &path) {
            m_breadcrumbs->setFilePath(path);
        });
    }

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

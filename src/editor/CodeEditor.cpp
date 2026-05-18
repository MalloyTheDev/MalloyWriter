#include "editor/CodeEditor.hpp"

#include <QPainter>
#include <QTextBlock>

namespace MalloyWriter::Editor {

LineNumberArea::LineNumberArea(CodeEditor *editor)
    : QWidget(editor)
    , m_editor(editor)
{
}

QSize LineNumberArea::sizeHint() const
{
    return {m_editor->lineNumberAreaWidth(), 0};
}

void LineNumberArea::paintEvent(QPaintEvent *event)
{
    m_editor->lineNumberAreaPaintEvent(event);
}

CodeEditor::CodeEditor(QWidget *parent)
    : QPlainTextEdit(parent)
    , m_lineNumberArea(new LineNumberArea(this))
{
    connect(this, &CodeEditor::blockCountChanged, this, &CodeEditor::updateLineNumberAreaWidth);
    connect(this, &CodeEditor::updateRequest, this, &CodeEditor::updateLineNumberArea);
    connect(this, &CodeEditor::cursorPositionChanged, this, &CodeEditor::highlightCurrentLine);

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();
}

int CodeEditor::lineNumberAreaWidth() const
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    return 10 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
}

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(m_lineNumberArea);
    painter.fillRect(event->rect(), QColor(35, 38, 45));

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            const int line = blockNumber + 1;
            QColor color(160, 168, 180);
            for (const auto &diagnostic : m_diagnostics) {
                if (diagnostic.line == line) {
                    color = diagnostic.severity == MalloyWriter::Platform::DiagnosticSeverity::Error
                        ? QColor(236, 92, 92)
                        : QColor(236, 184, 92);
                    break;
                }
            }

            painter.setPen(color);
            painter.drawText(0, top, m_lineNumberArea->width() - 4, fontMetrics().height(), Qt::AlignRight, QString::number(line));
        }

        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }
}

void CodeEditor::setDiagnostics(const QList<MalloyWriter::Platform::Diagnostic> &diagnostics)
{
    m_diagnostics = diagnostics;
    m_lineNumberArea->update();
}

bool CodeEditor::findNext(const QString &text, bool caseSensitive)
{
    if (text.isEmpty()) {
        return false;
    }

    QTextDocument::FindFlags flags;
    if (caseSensitive) {
        flags |= QTextDocument::FindCaseSensitively;
    }

    if (find(text, flags)) {
        return true;
    }

    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::Start);
    setTextCursor(cursor);
    return find(text, flags);
}

bool CodeEditor::replaceNext(const QString &findText, const QString &replaceText, bool caseSensitive)
{
    QTextCursor cursor = textCursor();
    if (!cursor.hasSelection() || cursor.selectedText() != findText) {
        if (!findNext(findText, caseSensitive)) {
            return false;
        }
        cursor = textCursor();
    }

    cursor.insertText(replaceText);
    return true;
}

void CodeEditor::goToLine(int line)
{
    const int target = qBound(1, line, blockCount());
    QTextBlock block = document()->findBlockByNumber(target - 1);
    if (!block.isValid()) {
        return;
    }

    QTextCursor cursor(block);
    setTextCursor(cursor);
    centerCursor();
}

void CodeEditor::resizeEvent(QResizeEvent *event)
{
    QPlainTextEdit::resizeEvent(event);
    const QRect contents = contentsRect();
    m_lineNumberArea->setGeometry(QRect(contents.left(), contents.top(), lineNumberAreaWidth(), contents.height()));
}

void CodeEditor::updateLineNumberAreaWidth(int)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void CodeEditor::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> selections;
    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;
        selection.format.setBackground(QColor(49, 53, 62));
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        selections.append(selection);
    }
    setExtraSelections(selections);
}

void CodeEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy != 0) {
        m_lineNumberArea->scroll(0, dy);
    } else {
        m_lineNumberArea->update(0, rect.y(), m_lineNumberArea->width(), rect.height());
    }

    if (rect.contains(viewport()->rect())) {
        updateLineNumberAreaWidth(0);
    }
}

}

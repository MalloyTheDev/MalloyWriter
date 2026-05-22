#pragma once

#include "platform/Diagnostic.hpp"

#include <QHash>
#include <QPlainTextEdit>

namespace MalloyWriter::Editor {

class LineNumberArea;

class CodeEditor : public QPlainTextEdit {
    Q_OBJECT

public:
    // Source-control marks for the gutter strip (populated by a future GitService).
    enum class LineMark { GitAdded, GitModified };

    explicit CodeEditor(QWidget *parent = nullptr);

    int lineNumberAreaWidth() const;
    void lineNumberAreaPaintEvent(QPaintEvent *event);
    void setDiagnostics(const QList<MalloyWriter::Platform::Diagnostic> &diagnostics);
    void setLineMarks(const QHash<int, LineMark> &marks);
    bool findNext(const QString &text, bool caseSensitive = false);
    bool replaceNext(const QString &findText, const QString &replaceText, bool caseSensitive = false);
    void goToLine(int line);

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLine();
    void updateLineNumberArea(const QRect &rect, int dy);

    LineNumberArea *m_lineNumberArea = nullptr;
    QList<MalloyWriter::Platform::Diagnostic> m_diagnostics;
    QHash<int, LineMark> m_lineMarks;
};

class LineNumberArea : public QWidget {
public:
    explicit LineNumberArea(CodeEditor *editor);

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    CodeEditor *m_editor = nullptr;
};

}

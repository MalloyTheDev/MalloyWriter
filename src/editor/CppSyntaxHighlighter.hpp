#pragma once

#include <QSyntaxHighlighter>
#include <QTextCharFormat>

namespace MalloyWriter::Editor {

// Lightweight C/C++ highlighter. Token rules are ported from MalloyIDE/syntax.jsx
// and colors come from the active Base::Theme (tk-* tokens). Block-comment state
// is tracked across lines via the block state machine.
class CppSyntaxHighlighter : public QSyntaxHighlighter {
    Q_OBJECT

public:
    explicit CppSyntaxHighlighter(QTextDocument *document = nullptr);

    // Rebuild formats from the active theme (call after a theme change).
    void refreshFormats();

protected:
    void highlightBlock(const QString &text) override;

private:
    enum BlockState { Normal = 0, InBlockComment = 1 };

    QTextCharFormat m_keyword;
    QTextCharFormat m_control;
    QTextCharFormat m_type;
    QTextCharFormat m_string;
    QTextCharFormat m_number;
    QTextCharFormat m_comment;
    QTextCharFormat m_function;
    QTextCharFormat m_variable;
    QTextCharFormat m_punct;
    QTextCharFormat m_preprocessor;
};

} // namespace MalloyWriter::Editor

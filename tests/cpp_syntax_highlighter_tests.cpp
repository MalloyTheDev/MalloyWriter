#include "base/Theme.hpp"
#include "editor/CppSyntaxHighlighter.hpp"

#include <QTextBlock>
#include <QTextDocument>
#include <QTextLayout>
#include <QtTest/QtTest>

using MalloyWriter::Editor::CppSyntaxHighlighter;

class CppSyntaxHighlighterTests : public QObject {
    Q_OBJECT

private:
    static bool hasFormatAt(const QTextBlock &block, int start, const QColor &color)
    {
        const auto formats = block.layout()->formats();
        for (const QTextLayout::FormatRange &range : formats) {
            if (range.start == start && range.format.foreground().color() == color) {
                return true;
            }
        }
        return false;
    }

private slots:
    void colorsKeywordAndType()
    {
        QTextDocument document;
        document.setPlainText(QStringLiteral("int QWidget;"));
        CppSyntaxHighlighter highlighter(&document);
        highlighter.rehighlight();

        const auto &theme = MalloyWriter::Base::Theme::active();
        const QTextBlock block = document.firstBlock();
        // "int" (cols 0-2) is a keyword; "QWidget" (col 4) is a type.
        QVERIFY(hasFormatAt(block, 0, theme.color(QStringLiteral("tk-kw"))));
        QVERIFY(hasFormatAt(block, 4, theme.color(QStringLiteral("tk-type"))));
    }

    void colorsLineComment()
    {
        QTextDocument document;
        document.setPlainText(QStringLiteral("// note"));
        CppSyntaxHighlighter highlighter(&document);
        highlighter.rehighlight();

        const auto &theme = MalloyWriter::Base::Theme::active();
        QVERIFY(hasFormatAt(document.firstBlock(), 0, theme.color(QStringLiteral("tk-comment"))));
    }
};

QTEST_MAIN(CppSyntaxHighlighterTests)

#include "cpp_syntax_highlighter_tests.moc"

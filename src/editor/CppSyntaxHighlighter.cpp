#include "editor/CppSyntaxHighlighter.hpp"

#include "base/Theme.hpp"

#include <QRegularExpression>
#include <QSet>

namespace MalloyWriter::Editor {

namespace {

const QSet<QString> &keywords()
{
    static const QSet<QString> set = {
        "auto", "bool", "char", "class", "const", "constexpr", "double", "enum", "explicit",
        "extern", "false", "float", "friend", "inline", "int", "long", "mutable", "namespace",
        "new", "noexcept", "nullptr", "operator", "private", "protected", "public", "short",
        "signed", "sizeof", "static", "struct", "template", "this", "true", "typedef",
        "typename", "union", "unsigned", "using", "virtual", "void", "volatile",
        "static_cast", "const_cast", "dynamic_cast", "reinterpret_cast", "override", "final",
    };
    return set;
}

const QSet<QString> &controlKeywords()
{
    static const QSet<QString> set = {
        "if", "else", "return", "for", "while", "switch", "case", "break",
        "continue", "throw", "try", "catch", "default", "do", "goto",
    };
    return set;
}

bool isLowercaseStdType(const QString &word)
{
    static const QRegularExpression re(QStringLiteral("^(u?int(8|16|32|64)_t|size_t|ptrdiff_t|wchar_t|char(8|16|32)_t)$"));
    return re.match(word).hasMatch();
}

bool isIdentifierStart(QChar c) { return c.isLetter() || c == QLatin1Char('_'); }
bool isIdentifierPart(QChar c) { return c.isLetterOrNumber() || c == QLatin1Char('_'); }

} // namespace

CppSyntaxHighlighter::CppSyntaxHighlighter(QTextDocument *document)
    : QSyntaxHighlighter(document)
{
    refreshFormats();
}

void CppSyntaxHighlighter::refreshFormats()
{
    const Base::Theme &theme = Base::Theme::active();
    auto color = [&theme](const char *token) { return theme.color(QString::fromLatin1(token)); };

    m_keyword.setForeground(color("tk-kw"));
    m_control.setForeground(color("tk-ctrl"));
    m_type.setForeground(color("tk-type"));
    m_string.setForeground(color("tk-string"));
    m_number.setForeground(color("tk-num"));
    m_comment.setForeground(color("tk-comment"));
    m_comment.setFontItalic(true);
    m_function.setForeground(color("tk-fn"));
    m_variable.setForeground(color("tk-var"));
    m_punct.setForeground(color("tk-punct"));
    m_preprocessor.setForeground(color("tk-pp"));
}

void CppSyntaxHighlighter::highlightBlock(const QString &text)
{
    const int n = text.length();
    int i = 0;

    // Continue a block comment opened on a previous line.
    if (previousBlockState() == InBlockComment) {
        const int end = text.indexOf(QStringLiteral("*/"));
        if (end == -1) {
            setFormat(0, n, m_comment);
            setCurrentBlockState(InBlockComment);
            return;
        }
        setFormat(0, end + 2, m_comment);
        i = end + 2;
    }

    setCurrentBlockState(Normal);

    while (i < n) {
        const QChar c = text.at(i);

        if (c == QLatin1Char(' ') || c == QLatin1Char('\t')) {
            ++i;
            continue;
        }

        // Line comment.
        if (c == QLatin1Char('/') && i + 1 < n && text.at(i + 1) == QLatin1Char('/')) {
            setFormat(i, n - i, m_comment);
            break;
        }

        // Block comment (possibly multi-line).
        if (c == QLatin1Char('/') && i + 1 < n && text.at(i + 1) == QLatin1Char('*')) {
            const int end = text.indexOf(QStringLiteral("*/"), i + 2);
            if (end == -1) {
                setFormat(i, n - i, m_comment);
                setCurrentBlockState(InBlockComment);
                break;
            }
            setFormat(i, end + 2 - i, m_comment);
            i = end + 2;
            continue;
        }

        // Preprocessor: '#' preceded only by whitespace on this line.
        if (c == QLatin1Char('#')) {
            bool leadingWhitespace = true;
            for (int k = 0; k < i; ++k) {
                if (text.at(k) != QLatin1Char(' ') && text.at(k) != QLatin1Char('\t')) {
                    leadingWhitespace = false;
                    break;
                }
            }
            if (leadingWhitespace) {
                setFormat(i, n - i, m_preprocessor);
                break;
            }
        }

        // String / char literals.
        if (c == QLatin1Char('"') || c == QLatin1Char('\'')) {
            const QChar quote = c;
            int j = i + 1;
            while (j < n && text.at(j) != quote) {
                if (text.at(j) == QLatin1Char('\\') && j + 1 < n) {
                    ++j;
                }
                ++j;
            }
            j = qMin(n, j + 1);
            setFormat(i, j - i, m_string);
            i = j;
            continue;
        }

        // Numbers.
        if (c.isDigit()) {
            int j = i;
            while (j < n && (text.at(j).isLetterOrNumber() || text.at(j) == QLatin1Char('.') || text.at(j) == QLatin1Char('_'))) {
                ++j;
            }
            setFormat(i, j - i, m_number);
            i = j;
            continue;
        }

        // Identifiers / keywords / types / functions.
        if (isIdentifierStart(c)) {
            int j = i;
            while (j < n && isIdentifierPart(text.at(j))) {
                ++j;
            }
            const QString word = text.mid(i, j - i);
            const QChar next = (j < n) ? text.at(j) : QChar();

            const QTextCharFormat *format = nullptr;
            if (controlKeywords().contains(word)) {
                format = &m_control;
            } else if (keywords().contains(word)) {
                format = &m_keyword;
            } else if (isLowercaseStdType(word) || word.at(0).isUpper()) {
                format = &m_type;
            } else if (next == QLatin1Char('(')) {
                format = &m_function;
            } else {
                format = &m_variable;
            }
            setFormat(i, j - i, *format);
            i = j;
            continue;
        }

        // Namespace operator.
        if (c == QLatin1Char(':') && i + 1 < n && text.at(i + 1) == QLatin1Char(':')) {
            setFormat(i, 2, m_punct);
            i += 2;
            continue;
        }

        // Punctuation.
        static const QString punct = QStringLiteral("{}[]();,.<>?:=+-*/%&|^!~");
        if (punct.contains(c)) {
            setFormat(i, 1, m_punct);
            ++i;
            continue;
        }

        ++i;
    }
}

} // namespace MalloyWriter::Editor

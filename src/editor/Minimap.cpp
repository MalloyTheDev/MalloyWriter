#include "editor/Minimap.hpp"

#include "base/Theme.hpp"
#include "editor/CodeEditor.hpp"

#include <QPainter>
#include <QScrollBar>

namespace MalloyWriter::Editor {

namespace {

constexpr int kMinimapWidth = 72;

} // namespace

Minimap::Minimap(CodeEditor *editor, QWidget *parent)
    : QWidget(parent)
    , m_editor(editor)
{
    setFixedWidth(kMinimapWidth);
    if (m_editor) {
        connect(m_editor, &QPlainTextEdit::textChanged, this, [this]() { rebuild(); });
        connect(m_editor->verticalScrollBar(), &QScrollBar::valueChanged, this, [this]() { update(); });
    }
    rebuild();
}

void Minimap::rebuild()
{
    m_rows.clear();
    if (!m_editor) {
        update();
        return;
    }

    const QStringList lines = m_editor->toPlainText().split(QLatin1Char('\n'));
    m_rows.reserve(lines.size());
    for (const QString &line : lines) {
        const QString trimmed = line.trimmed();
        Row row;
        row.widthFraction = qMin(1.0, trimmed.length() / 60.0);
        if (trimmed.startsWith(QStringLiteral("//")) || trimmed.startsWith(QStringLiteral("/*")) || trimmed.startsWith(QLatin1Char('*'))) {
            row.category = Comment;
        } else if (trimmed.startsWith(QStringLiteral("#"))) {
            row.category = Keyword;
        } else if (trimmed.contains(QLatin1Char('"'))) {
            row.category = String;
        } else if (trimmed.contains(QLatin1Char('('))) {
            row.category = Function;
        }
        m_rows.append(row);
    }
    update();
}

void Minimap::paintEvent(QPaintEvent *)
{
    const Base::Theme &theme = Base::Theme::active();
    QPainter p(this);
    p.fillRect(rect(), theme.color(QStringLiteral("bg-elev")));

    const int lineCount = m_rows.size();
    if (lineCount == 0) {
        return;
    }

    const double rowHeight = qMax(1.0, static_cast<double>(height()) / lineCount);
    const int usableWidth = width() - 8;

    auto colorFor = [&theme](Category c) {
        switch (c) {
        case Comment:  return theme.color(QStringLiteral("tk-comment"));
        case Keyword:  return theme.color(QStringLiteral("tk-kw"));
        case String:   return theme.color(QStringLiteral("tk-string"));
        case Function: return theme.color(QStringLiteral("tk-fn"));
        case Default:  break;
        }
        return theme.color(QStringLiteral("muted"));
    };

    p.setPen(Qt::NoPen);
    for (int i = 0; i < lineCount; ++i) {
        const double y = i * rowHeight;
        const int w = qMax(2, static_cast<int>(m_rows.at(i).widthFraction * usableWidth));
        QColor c = colorFor(m_rows.at(i).category);
        c.setAlphaF(0.65f);
        p.fillRect(QRectF(4, y, w, qMax(1.0, rowHeight - 0.5)), c);
    }

    // Viewport box from the editor's vertical scrollbar (line-based in QPlainTextEdit).
    if (m_editor) {
        QScrollBar *sb = m_editor->verticalScrollBar();
        const int first = sb->value();
        const int visible = qMax(1, sb->pageStep());
        const double top = first * rowHeight;
        const double boxHeight = visible * rowHeight;
        QColor box = theme.color(QStringLiteral("text"));
        box.setAlphaF(0.10f);
        p.fillRect(QRectF(0, top, width(), boxHeight), box);
        p.setPen(theme.color(QStringLiteral("border")));
        p.drawRect(QRectF(0, top, width() - 1, boxHeight));
    }
}

} // namespace MalloyWriter::Editor

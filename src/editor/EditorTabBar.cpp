#include "editor/EditorTabBar.hpp"

#include "base/Theme.hpp"
#include "workbench/Icon.hpp"

#include <QFileInfo>
#include <QMouseEvent>
#include <QPainter>
#include <QVariantMap>

namespace MalloyWriter::Editor {

namespace {

using Base::Theme;

struct FileFormat {
    QString icon;
    QColor color;
};

FileFormat formatForFile(const QString &fileName)
{
    const QString name = QFileInfo(fileName).fileName();
    const QString suffix = QFileInfo(fileName).suffix().toLower();
    auto oklch = [](double l, double c, double h) { return Base::oklchToColor({l, c, h}); };

    if (name.compare(QStringLiteral("CMakeLists.txt"), Qt::CaseInsensitive) == 0 || suffix == "cmake") {
        return {QStringLiteral("filecmake"), oklch(0.74, 0.13, 55)};
    }
    if (suffix == "cpp" || suffix == "cc" || suffix == "cxx" || suffix == "c") {
        return {QStringLiteral("filecpp"), oklch(0.70, 0.13, 240)};
    }
    if (suffix == "hpp" || suffix == "hxx" || suffix == "h" || suffix == "inl" || suffix == "ipp") {
        return {QStringLiteral("filehpp"), oklch(0.78, 0.12, 295)};
    }
    if (suffix == "md") {
        return {QStringLiteral("filemd"), oklch(0.78, 0.005, 245)};
    }
    if (suffix == "json") {
        return {QStringLiteral("filejson"), oklch(0.78, 0.13, 80)};
    }
    if (suffix == "git" || name.startsWith(QStringLiteral(".git"))) {
        return {QStringLiteral("filegit"), oklch(0.65, 0.16, 30)};
    }
    return {QStringLiteral("file"), oklch(0.70, 0.005, 245)};
}

constexpr int kIconSize = 14;
constexpr int kTrailing = 16; // close/dot hit area
constexpr int kPadding = 12;

} // namespace

EditorTabBar::EditorTabBar(QWidget *parent)
    : QTabBar(parent)
{
    setExpanding(false);
    setDrawBase(false);
    setMouseTracking(true);
    setMovable(true);
    setFocusPolicy(Qt::NoFocus);
}

void EditorTabBar::setTabMeta(QTabBar *bar, int index, const QString &fileName, bool modified, bool preview)
{
    QVariantMap meta;
    meta[QStringLiteral("file")] = fileName;
    meta[QStringLiteral("modified")] = modified;
    meta[QStringLiteral("preview")] = preview;
    bar->setTabData(index, meta);
}

QRect EditorTabBar::closeRect(const QRect &tabRect) const
{
    return QRect(tabRect.right() - kTrailing - 6, tabRect.center().y() - 7, 14, 14);
}

QSize EditorTabBar::tabSizeHint(int index) const
{
    const QString text = tabText(index);
    const int textWidth = fontMetrics().horizontalAdvance(text);
    const int width = kPadding + kIconSize + 6 + textWidth + 10 + kTrailing + kPadding;
    return {qMin(qMax(width, 120), 240), 35};
}

void EditorTabBar::paintEvent(QPaintEvent *)
{
    const Theme &theme = Theme::active();
    const QColor bg = theme.color(QStringLiteral("bg"));
    const QColor titlebar = theme.color(QStringLiteral("bg-titlebar"));
    const QColor surface = theme.color(QStringLiteral("surface"));
    const QColor borderSoft = theme.color(QStringLiteral("border-soft"));
    const QColor accent = theme.color(QStringLiteral("accent"));
    const QColor textColor = theme.color(QStringLiteral("text"));
    const QColor muted = theme.color(QStringLiteral("muted"));

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    for (int i = 0; i < count(); ++i) {
        const QRect rect = tabRect(i);
        const bool active = (i == currentIndex());
        const bool hovered = (i == m_hoveredTab);
        const QVariantMap meta = tabData(i).toMap();
        const bool modified = meta.value(QStringLiteral("modified")).toBool();
        const bool preview = meta.value(QStringLiteral("preview")).toBool();
        const FileFormat fmt = formatForFile(meta.value(QStringLiteral("file")).toString());

        // Background + separators.
        p.fillRect(rect, active ? bg : titlebar);
        if (hovered && !active) {
            p.fillRect(rect, surface);
        }
        p.setPen(borderSoft);
        p.drawLine(rect.topRight(), rect.bottomRight());
        if (active) {
            p.fillRect(QRect(rect.left(), rect.top(), rect.width(), 2), accent);
        }

        // File icon.
        const int iconY = rect.center().y() - kIconSize / 2;
        p.drawPixmap(rect.left() + kPadding, iconY, Workbench::Icon::pixmap(fmt.icon, kIconSize, fmt.color));

        // Name (elided, italic if preview).
        QFont font = p.font();
        font.setItalic(preview);
        p.setFont(font);
        const int textLeft = rect.left() + kPadding + kIconSize + 6;
        const int textRight = rect.right() - kTrailing - kPadding;
        const QRect textRect(textLeft, rect.top(), textRight - textLeft, rect.height());
        p.setPen(active ? textColor : muted);
        const QString elided = fontMetrics().elidedText(tabText(i), Qt::ElideRight, textRect.width());
        p.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, elided);
        font.setItalic(false);
        p.setFont(font);

        // Trailing: modified dot, or close "x" when hovered.
        const QRect close = closeRect(rect);
        if (modified && !(hovered)) {
            p.setPen(Qt::NoPen);
            p.setBrush(active ? textColor : muted);
            p.drawEllipse(close.center(), 4, 4);
        } else if (hovered) {
            p.drawPixmap(close.topLeft(), Workbench::Icon::pixmap(QStringLiteral("x"), 14, active ? textColor : muted));
        }
    }
}

void EditorTabBar::mouseMoveEvent(QMouseEvent *event)
{
    const int tab = tabAt(event->pos());
    if (tab != m_hoveredTab) {
        m_hoveredTab = tab;
        update();
    }
    QTabBar::mouseMoveEvent(event);
}

void EditorTabBar::mousePressEvent(QMouseEvent *event)
{
    const int tab = tabAt(event->pos());
    if (tab >= 0 && event->button() == Qt::LeftButton && closeRect(tabRect(tab)).contains(event->pos())) {
        emit tabCloseRequested(tab);
        return;
    }
    QTabBar::mousePressEvent(event);
}

void EditorTabBar::leaveEvent(QEvent *event)
{
    if (m_hoveredTab != -1) {
        m_hoveredTab = -1;
        update();
    }
    QTabBar::leaveEvent(event);
}

} // namespace MalloyWriter::Editor

#include "editor/Breadcrumbs.hpp"

#include "base/Theme.hpp"
#include "workbench/Icon.hpp"

#include <QHBoxLayout>
#include <QLabel>
#include <QLayoutItem>
#include <QPainter>

namespace MalloyWriter::Editor {

namespace {

QLabel *iconLabel(const QString &name, const QColor &color, QWidget *parent = nullptr)
{
    auto *label = new QLabel(parent);
    label->setPixmap(Workbench::Icon::pixmap(name, 12, color));
    return label;
}

QLabel *textLabel(const QString &text, const QColor &color, QWidget *parent = nullptr)
{
    auto *label = new QLabel(text, parent);
    QPalette pal = label->palette();
    pal.setColor(QPalette::WindowText, color);
    label->setPalette(pal);
    return label;
}

} // namespace

Breadcrumbs::Breadcrumbs(QWidget *parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("breadcrumbs"));
    setFixedHeight(28);
    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(12, 0, 12, 0);
    m_layout->setSpacing(5);
    m_layout->addStretch(1);
}

void Breadcrumbs::setFilePath(const QString &path)
{
    // Clear existing crumbs.
    while (QLayoutItem *item = m_layout->takeAt(0)) {
        if (QWidget *w = item->widget()) {
            w->deleteLater();
        }
        delete item;
    }

    const Base::Theme &theme = Base::Theme::active();
    const QColor crumb = theme.color(QStringLiteral("text-soft"));
    const QColor sep = theme.color(QStringLiteral("faint"));
    const QColor folder = theme.color(QStringLiteral("accent"));

    QString normalized = path;
    normalized.replace(QLatin1Char('\\'), QLatin1Char('/'));
    QStringList parts = normalized.split(QLatin1Char('/'), Qt::SkipEmptyParts);
    if (parts.isEmpty()) {
        m_layout->addStretch(1);
        return;
    }

    // Trailing segments only (keep the row compact).
    const int maxSegments = 4;
    if (parts.size() > maxSegments) {
        parts = parts.mid(parts.size() - maxSegments);
    }

    for (int i = 0; i < parts.size(); ++i) {
        const bool last = (i == parts.size() - 1);
        if (i > 0) {
            m_layout->addWidget(iconLabel(QStringLiteral("chevRight"), sep));
        }
        m_layout->addWidget(iconLabel(last ? QStringLiteral("file") : QStringLiteral("folder"),
                                      last ? crumb : folder));
        m_layout->addWidget(textLabel(parts.at(i), crumb));
    }
    m_layout->addStretch(1);
}

void Breadcrumbs::paintEvent(QPaintEvent *)
{
    const Base::Theme &theme = Base::Theme::active();
    QPainter p(this);
    p.fillRect(rect(), theme.color(QStringLiteral("bg")));
    p.setPen(theme.color(QStringLiteral("border-soft")));
    p.drawLine(rect().bottomLeft(), rect().bottomRight());
}

} // namespace MalloyWriter::Editor

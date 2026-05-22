#include "workbench/Sidebar.hpp"

#include "base/Theme.hpp"
#include "workbench/Icon.hpp"

#include <QHBoxLayout>
#include <QLabel>
#include <QLayoutItem>
#include <QStackedWidget>
#include <QToolButton>
#include <QVBoxLayout>

namespace MalloyWriter::Workbench {

namespace {

using Base::Theme;

struct ViewMeta { const char *id; const char *title; const char *icon; const char *blurb; };

const ViewMeta kViews[] = {
    {"explorer", "EXPLORER",       "explorer", "Open a folder to browse its files."},
    {"search",   "SEARCH",         "search",   "Search across files in the workspace."},
    {"scm",      "SOURCE CONTROL", "scm",      "Git changes, commits, and history."},
    {"run",      "RUN AND DEBUG",  "run",      "Launch targets, breakpoints, and variables."},
    {"ext",      "EXTENSIONS",     "ext",      "Installed and recommended extensions."},
    {"test",     "TESTING",        "test",     "No tests have been found in this workspace."},
    {"ai",       "MW ASSIST",      "ai",       "Local, preview-first coding assistant."},
    {"remote",   "REMOTE",         "remote",   "Not connected to a remote."},
};

QToolButton *makeIconButton(const QString &iconName, const QString &tooltip, QWidget *parent)
{
    auto *button = new QToolButton(parent);
    button->setObjectName(QStringLiteral("iconBtn"));
    button->setToolTip(tooltip);
    button->setCursor(Qt::PointingHandCursor);
    button->setIcon(Icon::icon(iconName, 14, Theme::active().color(QStringLiteral("text-soft"))));
    button->setIconSize(QSize(14, 14));
    button->setFixedSize(22, 22);
    button->setAutoRaise(true);
    return button;
}

// Centered icon + title + subtitle, like the prototype's empty states.
QWidget *makePlaceholder(const ViewMeta &meta, QWidget *parent)
{
    auto *page = new QWidget(parent);
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(24, 40, 24, 24);
    layout->setSpacing(10);
    layout->addStretch(1);

    auto *iconLabel = new QLabel(page);
    iconLabel->setAlignment(Qt::AlignCenter);
    iconLabel->setPixmap(Icon::pixmap(QString::fromLatin1(meta.icon), 28,
                                      Theme::active().color(QStringLiteral("faint"))));
    layout->addWidget(iconLabel);

    auto *blurb = new QLabel(QString::fromLatin1(meta.blurb), page);
    blurb->setObjectName(QStringLiteral("placeholderText"));
    blurb->setWordWrap(true);
    blurb->setAlignment(Qt::AlignCenter);
    layout->addWidget(blurb);
    layout->addStretch(2);
    return page;
}

} // namespace

Sidebar::Sidebar(QWidget *parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("sidebar"));
    setFixedWidth(280);

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    // Header: title + action buttons.
    auto *header = new QWidget(this);
    header->setObjectName(QStringLiteral("sidebarHeader"));
    header->setFixedHeight(36);
    auto *headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(14, 0, 8, 0);
    headerLayout->setSpacing(2);

    m_title = new QLabel(QStringLiteral("EXPLORER"), header);
    m_title->setObjectName(QStringLiteral("sidebarTitle"));
    headerLayout->addWidget(m_title);
    headerLayout->addStretch(1);

    auto *actions = new QWidget(header);
    m_actions = new QHBoxLayout(actions);
    m_actions->setContentsMargins(0, 0, 0, 0);
    m_actions->setSpacing(1);
    headerLayout->addWidget(actions);

    root->addWidget(header);

    // Body: one page per view.
    m_stack = new QStackedWidget(this);
    m_stack->setObjectName(QStringLiteral("sidebarBody"));
    for (const ViewMeta &meta : kViews) {
        const int index = m_stack->addWidget(makePlaceholder(meta, m_stack));
        m_pageIndex.insert(QString::fromLatin1(meta.id), index);
    }
    root->addWidget(m_stack, 1);

    rebuildHeaderActions(QStringLiteral("explorer"));
}

void Sidebar::setViewWidget(const QString &id, QWidget *widget)
{
    const auto it = m_pageIndex.constFind(id);
    if (it == m_pageIndex.constEnd() || !widget) {
        return;
    }
    QWidget *old = m_stack->widget(it.value());
    const bool wasCurrent = (m_stack->currentIndex() == it.value());
    m_stack->removeWidget(old);
    old->deleteLater();
    const int newIndex = m_stack->insertWidget(it.value(), widget);
    m_pageIndex.insert(id, newIndex);
    if (wasCurrent) {
        m_stack->setCurrentIndex(newIndex);
    }
}

void Sidebar::setView(const QString &id)
{
    const auto it = m_pageIndex.constFind(id);
    if (it == m_pageIndex.constEnd()) {
        return;
    }
    m_stack->setCurrentIndex(it.value());
    for (const ViewMeta &meta : kViews) {
        if (id == QLatin1String(meta.id)) {
            m_title->setText(QString::fromLatin1(meta.title));
            break;
        }
    }
    rebuildHeaderActions(id);
}

void Sidebar::rebuildHeaderActions(const QString &id)
{
    while (QLayoutItem *item = m_actions->takeAt(0)) {
        if (QWidget *w = item->widget()) {
            w->deleteLater();
        }
        delete item;
    }

    QWidget *owner = m_actions->parentWidget();
    if (id == QLatin1String("explorer")) {
        m_actions->addWidget(makeIconButton(QStringLiteral("file"), QStringLiteral("New File"), owner));
        m_actions->addWidget(makeIconButton(QStringLiteral("folder"), QStringLiteral("New Folder"), owner));
        m_actions->addWidget(makeIconButton(QStringLiteral("refresh"), QStringLiteral("Refresh"), owner));
        m_actions->addWidget(makeIconButton(QStringLiteral("collapse"), QStringLiteral("Collapse Folders"), owner));
    } else if (id == QLatin1String("scm")) {
        m_actions->addWidget(makeIconButton(QStringLiteral("refresh"), QStringLiteral("Refresh"), owner));
        m_actions->addWidget(makeIconButton(QStringLiteral("more"), QStringLiteral("More"), owner));
    } else {
        m_actions->addWidget(makeIconButton(QStringLiteral("more"), QStringLiteral("More"), owner));
    }
}

} // namespace MalloyWriter::Workbench

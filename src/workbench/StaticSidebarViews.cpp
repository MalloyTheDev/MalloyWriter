#include "workbench/StaticSidebarViews.hpp"

#include "base/Theme.hpp"
#include "workbench/Icon.hpp"

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>

namespace MalloyWriter::Workbench {

namespace {

using Base::Theme;

QColor token(const char *name) { return Theme::active().color(QString::fromLatin1(name)); }

QLabel *header(const QString &title, QWidget *parent)
{
    auto *label = new QLabel(title, parent);
    label->setObjectName(QStringLiteral("groupHeader"));
    label->setContentsMargins(14, 12, 8, 6);
    return label;
}

QLabel *coloredLabel(const QString &text, const QColor &color, int pixelSize, bool bold, QWidget *parent)
{
    auto *label = new QLabel(text, parent);
    QPalette pal = label->palette();
    pal.setColor(QPalette::WindowText, color);
    label->setPalette(pal);
    QFont font = label->font();
    font.setPixelSize(pixelSize);
    font.setBold(bold);
    label->setFont(font);
    return label;
}

// A small rounded status badge ("M", "U", …).
QLabel *badge(const QString &text, const QColor &fg, const QColor &bg, QWidget *parent)
{
    auto *label = new QLabel(text, parent);
    label->setAlignment(Qt::AlignCenter);
    label->setFixedSize(16, 16);
    QPalette pal = label->palette();
    pal.setColor(QPalette::WindowText, fg);
    label->setPalette(pal);
    QFont font = label->font();
    font.setPixelSize(10);
    font.setBold(true);
    label->setFont(font);
    label->setStyleSheet(QStringLiteral("background-color: %1; border-radius: 4px;").arg(bg.name()));
    return label;
}

QWidget *makeScrollBody(QWidget *owner, QVBoxLayout *&outLayout)
{
    auto *scroll = new QScrollArea(owner);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    auto *body = new QWidget(scroll);
    outLayout = new QVBoxLayout(body);
    outLayout->setContentsMargins(0, 0, 0, 8);
    outLayout->setSpacing(0);
    scroll->setWidget(body);
    return scroll;
}

} // namespace

// ───────────────────────── Source Control ─────────────────────────
ScmView::ScmView(QWidget *parent)
    : QWidget(parent)
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(10, 10, 8, 6);
    root->setSpacing(8);

    auto *message = new QPlainTextEdit(this);
    message->setPlaceholderText(tr("Message (commit on the command line for now)"));
    message->setPlainText(QStringLiteral("workbench: wire OutputPanel to BuildService::outputReceived"));
    message->setFixedHeight(58);
    root->addWidget(message);

    auto *commitRow = new QHBoxLayout;
    auto *commit = new QPushButton(Icon::icon(QStringLiteral("check"), 12, token("accent-fg")), tr("Commit"), this);
    commit->setEnabled(false);
    commit->setToolTip(tr("Git integration arrives in a later phase"));
    commitRow->addWidget(commit, 1);
    root->addLayout(commitRow);

    QVBoxLayout *body = nullptr;
    root->addWidget(makeScrollBody(this, body), 1);

    body->addWidget(header(tr("CHANGES"), this));
    struct Change { const char *name; const char *path; const char *status; bool untracked; };
    const Change changes[] = {
        {"MainWindow.cpp", "src/workbench", "M", false},
        {"BuildService.cpp", "src/platform", "M", false},
        {"CMakeLists.txt", "/", "M", false},
        {"SearchService.cpp", "src/platform", "U", true},
    };
    for (const Change &change : changes) {
        auto *row = new QWidget(this);
        auto *rowLayout = new QHBoxLayout(row);
        rowLayout->setContentsMargins(14, 2, 10, 2);
        rowLayout->setSpacing(6);
        const FileType type = Icon::fileType(QString::fromLatin1(change.name));
        auto *icon = new QLabel(row);
        icon->setPixmap(Icon::pixmap(type.iconName, 14, type.color));
        rowLayout->addWidget(icon);
        rowLayout->addWidget(coloredLabel(QString::fromLatin1(change.name), token("text-soft"), 13, false, row));
        rowLayout->addWidget(coloredLabel(QString::fromLatin1(change.path), token("muted"), 11, false, row));
        rowLayout->addStretch(1);
        const QColor badgeColor = change.untracked ? token("ok") : token("mod");
        rowLayout->addWidget(badge(QString::fromLatin1(change.status), token("accent-fg"), badgeColor, row));
        body->addWidget(row);
    }

    body->addWidget(header(tr("COMMITS"), this));
    struct Commit { const char *message; const char *hash; };
    const Commit commits[] = {
        {"platform: recursive workspace search service", "a3f8e21"},
        {"editor: gutter diagnostic markers from clangd", "1c0d3a4"},
        {"workbench: keyboard-first command palette", "f12a009"},
    };
    for (const Commit &entry : commits) {
        auto *row = new QWidget(this);
        auto *rowLayout = new QHBoxLayout(row);
        rowLayout->setContentsMargins(14, 2, 10, 2);
        rowLayout->setSpacing(6);
        auto *icon = new QLabel(row);
        icon->setPixmap(Icon::pixmap(QStringLiteral("scm"), 14, token("tk-fn")));
        rowLayout->addWidget(icon);
        rowLayout->addWidget(coloredLabel(QString::fromLatin1(entry.message), token("text-soft"), 12, false, row), 1);
        rowLayout->addWidget(coloredLabel(QString::fromLatin1(entry.hash), token("faint"), 11, false, row));
        body->addWidget(row);
    }
    body->addStretch(1);
}

// ───────────────────────── Extensions ─────────────────────────
namespace {

QWidget *makeExtensionCard(const QString &initial, const QColor &color, const QString &name,
                           const QString &version, const QString &description, const QString &meta,
                           const QString &action, QWidget *parent)
{
    auto *card = new QWidget(parent);
    auto *cardLayout = new QHBoxLayout(card);
    cardLayout->setContentsMargins(14, 6, 10, 6);
    cardLayout->setSpacing(8);

    auto *iconBox = new QLabel(initial, card);
    iconBox->setAlignment(Qt::AlignCenter);
    iconBox->setFixedSize(36, 36);
    iconBox->setStyleSheet(QStringLiteral("background-color: %1; color: #15181c; border-radius: 6px; font-weight: 700;").arg(color.name()));
    cardLayout->addWidget(iconBox, 0, Qt::AlignTop);

    auto *textColumn = new QVBoxLayout;
    textColumn->setSpacing(2);
    auto *titleRow = new QHBoxLayout;
    titleRow->setSpacing(6);
    titleRow->addWidget(coloredLabel(name, token("text"), 13, true, card));
    titleRow->addWidget(coloredLabel(version, token("faint"), 11, false, card));
    titleRow->addStretch(1);
    textColumn->addLayout(titleRow);
    auto *desc = coloredLabel(description, token("text-soft"), 12, false, card);
    desc->setWordWrap(true);
    textColumn->addWidget(desc);
    textColumn->addWidget(coloredLabel(meta, token("muted"), 11, false, card));
    cardLayout->addLayout(textColumn, 1);

    auto *actionButton = new QPushButton(action, card);
    actionButton->setFixedHeight(22);
    cardLayout->addWidget(actionButton, 0, Qt::AlignTop);
    return card;
}

} // namespace

ExtensionsView::ExtensionsView(QWidget *parent)
    : QWidget(parent)
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(10, 10, 8, 6);
    root->setSpacing(8);

    auto *search = new QLineEdit(this);
    search->setPlaceholderText(tr("Search Extensions in Marketplace"));
    root->addWidget(search);

    QVBoxLayout *body = nullptr;
    root->addWidget(makeScrollBody(this, body), 1);

    auto oklch = [](double l, double c, double h) { return Base::oklchToColor({l, c, h}); };

    body->addWidget(header(tr("INSTALLED"), this));
    body->addWidget(makeExtensionCard("C", oklch(0.72, 0.13, 215), QStringLiteral("clangd"), QStringLiteral("0.1.31"),
                                      tr("C/C++ completion, navigation, and diagnostics via clangd."),
                                      QStringLiteral("LLVM · 3.2M ★ 4.7"), tr("Installed"), this));
    body->addWidget(makeExtensionCard("△", oklch(0.7, 0.13, 30), QStringLiteral("CMake Tools"), QStringLiteral("1.18.2"),
                                      tr("Configure, build, debug and package CMake projects."),
                                      QStringLiteral("MalloyWriter · 12.4M ★ 4.5"), tr("Installed"), this));
    body->addWidget(makeExtensionCard("G", oklch(0.62, 0.18, 30), QStringLiteral("Git Lens+"), QStringLiteral("14.7.0"),
                                      tr("Supercharge Git in the editor — blame, history, comparisons."),
                                      QStringLiteral("git-lens · 28.1M ★ 4.9"), tr("Update"), this));

    body->addWidget(header(tr("RECOMMENDED"), this));
    body->addWidget(makeExtensionCard("R", oklch(0.68, 0.14, 280), QStringLiteral("REST Client"), QStringLiteral("0.25.1"),
                                      tr("Send HTTP requests and view responses in the editor."),
                                      QStringLiteral("humao · 5.4M ★ 4.7"), tr("Install"), this));
    body->addWidget(makeExtensionCard("D", oklch(0.7, 0.13, 215), QStringLiteral("Docker"), QStringLiteral("1.29.0"),
                                      tr("Build, manage, and deploy containerized applications."),
                                      QStringLiteral("MalloyWriter · 21.5M ★ 4.6"), tr("Install"), this));
    body->addStretch(1);
}

// ───────────────────────── MW Assist ─────────────────────────
namespace {

QWidget *makeChatBubble(const QString &role, const QColor &roleColor, const QString &text, QWidget *parent)
{
    auto *bubble = new QWidget(parent);
    bubble->setStyleSheet(QStringLiteral("background-color: %1; border: 1px solid %2; border-radius: 5px;")
                              .arg(token("bg").name(), token("border").name()));
    auto *layout = new QVBoxLayout(bubble);
    layout->setContentsMargins(10, 8, 10, 8);
    layout->setSpacing(4);
    layout->addWidget(coloredLabel(role, roleColor, 10, true, bubble));
    auto *body = coloredLabel(text, token("text-soft"), 13, false, bubble);
    body->setWordWrap(true);
    layout->addWidget(body);
    return bubble;
}

} // namespace

AssistView::AssistView(QWidget *parent)
    : QWidget(parent)
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(12, 12, 12, 12);
    root->setSpacing(10);

    auto *titleRow = new QHBoxLayout;
    titleRow->setSpacing(8);
    auto *aiIcon = new QLabel(this);
    aiIcon->setPixmap(Icon::pixmap(QStringLiteral("ai"), 16, token("accent")));
    titleRow->addWidget(aiIcon);
    titleRow->addWidget(coloredLabel(tr("MW Assist"), token("text"), 13, true, this));
    titleRow->addStretch(1);
    titleRow->addWidget(coloredLabel(QStringLiteral("local · qwen2.5-coder-14b"), token("muted"), 10, false, this));
    root->addLayout(titleRow);

    root->addWidget(makeChatBubble(tr("YOU"), token("muted"),
                                   tr("Explain the connect() on line 44 — why does clang-tidy warn?"), this));
    root->addWidget(makeChatBubble(tr("MW ASSIST"), token("accent"),
                                   tr("recentFilesChanged is overloaded, so the function-pointer cast is "
                                      "ambiguous. Wrap it with qOverload<>() to disambiguate."), this));
    root->addStretch(1);

    auto *input = new QPlainTextEdit(this);
    input->setPlaceholderText(tr("Ask MW Assist about the open file…"));
    input->setFixedHeight(64);
    root->addWidget(input);

    auto *footer = new QHBoxLayout;
    footer->addWidget(coloredLabel(tr("Disabled by default · enable a provider in Settings"), token("muted"), 11, false, this), 1);
    auto *send = new QPushButton(tr("Send"), this);
    send->setEnabled(false);
    footer->addWidget(send);
    root->addLayout(footer);
}

} // namespace MalloyWriter::Workbench

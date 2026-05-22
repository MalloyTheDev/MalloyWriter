#include "workbench/WelcomeScreen.hpp"

#include "base/AppInfo.hpp"
#include "base/Theme.hpp"
#include "workbench/Icon.hpp"

#include <functional>

#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QLayoutItem>
#include <QMouseEvent>
#include <QPushButton>
#include <QVBoxLayout>

namespace MalloyWriter::Workbench {

namespace {

using Base::Theme;

// A clickable Start card (icon + title + description).
class StartCard : public QFrame {
public:
    StartCard(const QString &iconName, const QString &title, const QString &description,
              std::function<void()> onClick, QWidget *parent = nullptr)
        : QFrame(parent)
        , m_onClick(std::move(onClick))
    {
        setObjectName(QStringLiteral("startCard"));
        setAttribute(Qt::WA_Hover, true);
        setCursor(Qt::PointingHandCursor);
        auto *layout = new QHBoxLayout(this);
        layout->setContentsMargins(12, 10, 12, 10);
        layout->setSpacing(10);

        auto *icon = new QLabel(this);
        icon->setPixmap(Icon::pixmap(iconName, 16, Theme::active().color(QStringLiteral("accent"))));
        layout->addWidget(icon, 0, Qt::AlignTop);

        auto *text = new QVBoxLayout;
        text->setSpacing(2);
        auto *titleLabel = new QLabel(title, this);
        QFont titleFont = titleLabel->font();
        titleFont.setWeight(QFont::DemiBold);
        titleLabel->setFont(titleFont);
        text->addWidget(titleLabel);
        auto *descLabel = new QLabel(description, this);
        QPalette pal = descLabel->palette();
        pal.setColor(QPalette::WindowText, Theme::active().color(QStringLiteral("muted")));
        descLabel->setPalette(pal);
        text->addWidget(descLabel);
        layout->addLayout(text, 1);
    }

protected:
    void mousePressEvent(QMouseEvent *event) override
    {
        if (event->button() == Qt::LeftButton && m_onClick) {
            m_onClick();
        }
    }

private:
    std::function<void()> m_onClick;
};

} // namespace

WelcomeScreen::WelcomeScreen(QWidget *parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("welcomeScreen"));
    auto *outer = new QHBoxLayout(this);
    outer->addStretch(1);

    auto *column = new QVBoxLayout;
    column->setContentsMargins(0, 48, 0, 48);
    column->setSpacing(8);
    column->setAlignment(Qt::AlignTop);

    auto *title = new QLabel(QStringLiteral("MalloyWriter"), this);
    title->setObjectName(QStringLiteral("welcomeTitle"));
    QFont titleFont = title->font();
    titleFont.setPixelSize(34);
    titleFont.setWeight(QFont::Bold);
    title->setFont(titleFont);
    column->addWidget(title);

    auto *version = new QLabel(tr("v%1 · native · craft-first").arg(MalloyWriter::Base::appVersion()), this);
    QPalette versionPal = version->palette();
    versionPal.setColor(QPalette::WindowText, Theme::active().color(QStringLiteral("muted")));
    version->setPalette(versionPal);
    column->addWidget(version);

    auto *tagline = new QLabel(tr("A native IDE for C++, Qt, and CMake — keyboard-driven and AI-assisted."), this);
    tagline->setWordWrap(true);
    QPalette taglinePal = tagline->palette();
    taglinePal.setColor(QPalette::WindowText, Theme::active().color(QStringLiteral("text-soft")));
    tagline->setPalette(taglinePal);
    column->addWidget(tagline);
    column->addSpacing(20);

    // Start cards.
    auto *startHeader = new QLabel(tr("Start"), this);
    QFont startFont = startHeader->font();
    startFont.setWeight(QFont::DemiBold);
    startHeader->setFont(startFont);
    column->addWidget(startHeader);

    column->addWidget(new StartCard(QStringLiteral("folder"), tr("Open Folder"), tr("Mount a workspace"),
                                    [this]() { emit openFolderRequested(); }, this));
    column->addWidget(new StartCard(QStringLiteral("file"), tr("Open File"), tr("Open a single file"),
                                    [this]() { emit openFileRequested(); }, this));
    column->addWidget(new StartCard(QStringLiteral("cmdK"), tr("All Commands"), tr("The whole IDE, one search away"),
                                    [this]() { emit commandPaletteRequested(); }, this));
    column->addSpacing(20);

    auto *recentHeader = new QLabel(tr("Recent"), this);
    recentHeader->setFont(startFont);
    column->addWidget(recentHeader);

    auto *recentContainer = new QWidget(this);
    m_recentLayout = new QVBoxLayout(recentContainer);
    m_recentLayout->setContentsMargins(0, 0, 0, 0);
    m_recentLayout->setSpacing(0);
    column->addWidget(recentContainer);
    column->addStretch(1);

    outer->addLayout(column);
    outer->addStretch(1);
    outer->setStretch(0, 1);
    outer->setStretch(1, 2);
    outer->setStretch(2, 1);

    setRecentWorkspaces({});
}

void WelcomeScreen::setRecentWorkspaces(const QStringList &paths)
{
    while (QLayoutItem *item = m_recentLayout->takeAt(0)) {
        if (QWidget *w = item->widget()) {
            w->deleteLater();
        }
        delete item;
    }

    if (paths.isEmpty()) {
        auto *empty = new QLabel(tr("No recent workspaces yet."), m_recentLayout->parentWidget());
        QPalette pal = empty->palette();
        pal.setColor(QPalette::WindowText, Theme::active().color(QStringLiteral("muted")));
        empty->setPalette(pal);
        empty->setContentsMargins(6, 4, 0, 0);
        m_recentLayout->addWidget(empty);
        return;
    }

    for (const QString &path : paths) {
        auto *row = new QPushButton(m_recentLayout->parentWidget());
        row->setObjectName(QStringLiteral("recentRow"));
        row->setCursor(Qt::PointingHandCursor);
        row->setFlat(true);
        const QString name = QFileInfo(path).fileName();
        row->setText(name.isEmpty() ? path : QStringLiteral("%1   —   %2").arg(name, path));
        row->setToolTip(path);
        connect(row, &QPushButton::clicked, this, [this, path]() { emit openRecentRequested(path); });
        m_recentLayout->addWidget(row);
    }
}

} // namespace MalloyWriter::Workbench

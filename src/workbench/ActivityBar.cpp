#include "workbench/ActivityBar.hpp"

#include "base/Theme.hpp"
#include "workbench/Icon.hpp"

#include <QAbstractButton>
#include <QButtonGroup>
#include <QPainter>
#include <QVBoxLayout>

namespace MalloyWriter::Workbench {

namespace {

using Base::Theme;

// A single activity-bar button: custom-painted icon + active accent bar + badge.
class ActivityButton : public QAbstractButton {
public:
    ActivityButton(QString id, QString iconName, const QString &tooltip, QWidget *parent)
        : QAbstractButton(parent)
        , m_id(std::move(id))
        , m_iconName(std::move(iconName))
    {
        setCheckable(true);
        setCursor(Qt::PointingHandCursor);
        setToolTip(tooltip);
        setFixedSize(48, 42);
    }

    const QString &viewId() const { return m_id; }

    void setBadge(const QString &text, bool warn)
    {
        m_badge = text;
        m_badgeWarn = warn;
        update();
    }

protected:
    void enterEvent(QEnterEvent *) override { m_hover = true; update(); }
    void leaveEvent(QEvent *) override { m_hover = false; update(); }

    void paintEvent(QPaintEvent *) override
    {
        const Theme &theme = Theme::active();
        const bool lit = isChecked() || m_hover;
        const QColor iconColor = lit ? theme.color(QStringLiteral("text"))
                                     : theme.color(QStringLiteral("muted"));

        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing, true);

        if (isChecked()) {
            p.fillRect(QRect(0, 0, 2, height()), theme.color(QStringLiteral("accent")));
        }

        const int iconSize = 20;
        const QPixmap pm = Icon::pixmap(m_iconName, iconSize, iconColor);
        const QPointF topLeft((width() - iconSize) / 2.0, (height() - iconSize) / 2.0);
        p.drawPixmap(topLeft, pm);

        if (!m_badge.isEmpty()) {
            const QColor bg = m_badgeWarn ? theme.color(QStringLiteral("warn"))
                                          : theme.color(QStringLiteral("accent"));
            const QColor fg = m_badgeWarn ? QColor(0x33, 0x2a, 0x12)
                                          : theme.color(QStringLiteral("accent-fg"));
            QFont f = font();
            f.setPixelSize(9);
            f.setBold(true);
            p.setFont(f);
            const int w = std::max(14, p.fontMetrics().horizontalAdvance(m_badge) + 6);
            const QRectF badge(width() - w - 4, height() - 18, w, 13);
            p.setPen(Qt::NoPen);
            p.setBrush(bg);
            p.drawRoundedRect(badge, 6.5, 6.5);
            p.setPen(fg);
            p.drawText(badge, Qt::AlignCenter, m_badge);
        }
    }

private:
    QString m_id;
    QString m_iconName;
    QString m_badge;
    bool m_badgeWarn = false;
    bool m_hover = false;
};

} // namespace

ActivityBar::ActivityBar(QWidget *parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("activityBar"));
    setFixedWidth(48);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 2, 0, 2);
    layout->setSpacing(0);

    m_group = new QButtonGroup(this);
    m_group->setExclusive(true);

    struct Entry { const char *id; const char *icon; const char *label; const char *badge; bool warn; };
    const Entry views[] = {
        {"explorer", "explorer", "Explorer",       nullptr, false},
        {"search",   "search",   "Search",         nullptr, false},
        {"scm",      "scm",      "Source Control", "5",     false},
        {"run",      "run",      "Run and Debug",  nullptr, false},
        {"ext",      "ext",      "Extensions",     "1",     true},
        {"test",     "test",     "Testing",        nullptr, false},
        {"ai",       "ai",       "MW Assist",      nullptr, false},
        {"remote",   "remote",   "Remote",         nullptr, false},
    };

    for (const Entry &e : views) {
        auto *button = new ActivityButton(QString::fromLatin1(e.id), QString::fromLatin1(e.icon),
                                          QString::fromLatin1(e.label), this);
        if (e.badge) {
            button->setBadge(QString::fromLatin1(e.badge), e.warn);
        }
        m_group->addButton(button);
        layout->addWidget(button);
        connect(button, &QAbstractButton::clicked, this, [this, button]() {
            const QString id = button->viewId();
            if (m_currentView != id) {
                m_currentView = id;
                emit viewChanged(id);
            }
        });
    }

    layout->addStretch(1);

    auto *accounts = new ActivityButton(QStringLiteral("accounts"), QStringLiteral("acc"),
                                        QStringLiteral("Accounts"), this);
    accounts->setCheckable(false);
    layout->addWidget(accounts);

    auto *settings = new ActivityButton(QStringLiteral("settings"), QStringLiteral("settings"),
                                        QStringLiteral("Settings"), this);
    settings->setCheckable(false);
    layout->addWidget(settings);
    connect(settings, &QAbstractButton::clicked, this, &ActivityBar::settingsRequested);

    setCurrentView(QStringLiteral("explorer"));
}

void ActivityBar::setCurrentView(const QString &id)
{
    // Only ActivityButtons are added to m_group, so the cast is safe.
    for (QAbstractButton *button : m_group->buttons()) {
        auto *ab = static_cast<ActivityButton *>(button);
        if (ab->viewId() == id) {
            ab->setChecked(true);
            if (m_currentView != id) {
                m_currentView = id;
                emit viewChanged(id);
            }
            return;
        }
    }
}

} // namespace MalloyWriter::Workbench

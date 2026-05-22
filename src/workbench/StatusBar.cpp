#include "workbench/StatusBar.hpp"

#include "base/Theme.hpp"
#include "workbench/Icon.hpp"

#include <functional>

#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>

namespace MalloyWriter::Workbench {

namespace {

using Base::Theme;

QPixmap dotPixmap(const QColor &color)
{
    QPixmap pm(8, 8);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setPen(Qt::NoPen);
    p.setBrush(color);
    p.drawEllipse(1, 1, 6, 6);
    return pm;
}

// A hoverable, optionally-clickable status-bar cell holding icons/text/dots.
class StatusItem : public QWidget {
public:
    explicit StatusItem(QWidget *parent = nullptr)
        : QWidget(parent)
    {
        setCursor(Qt::PointingHandCursor);
        m_layout = new QHBoxLayout(this);
        m_layout->setContentsMargins(8, 0, 8, 0);
        m_layout->setSpacing(5);
    }

    QLabel *addIcon(const QString &name, const QColor &color, int size = 12)
    {
        auto *label = new QLabel(this);
        label->setPixmap(Icon::pixmap(name, size, color));
        m_layout->addWidget(label);
        return label;
    }

    QLabel *addText(const QString &text, const QColor &color)
    {
        auto *label = new QLabel(text, this);
        QPalette pal = label->palette();
        pal.setColor(QPalette::WindowText, color);
        label->setPalette(pal);
        QFont f = label->font();
        f.setPixelSize(12);
        label->setFont(f);
        m_layout->addWidget(label);
        return label;
    }

    QLabel *addDot(const QColor &color)
    {
        auto *label = new QLabel(this);
        label->setPixmap(dotPixmap(color));
        m_layout->addWidget(label);
        return label;
    }

    void setOnClick(std::function<void()> handler) { m_onClick = std::move(handler); }

protected:
    void enterEvent(QEnterEvent *) override { m_hover = true; update(); }
    void leaveEvent(QEvent *) override { m_hover = false; update(); }

    void paintEvent(QPaintEvent *) override
    {
        if (m_hover) {
            QPainter p(this);
            p.fillRect(rect(), Theme::active().color(QStringLiteral("surface")));
        }
    }

    void mousePressEvent(QMouseEvent *event) override
    {
        if (event->button() == Qt::LeftButton && m_onClick) {
            m_onClick();
        }
    }

private:
    QHBoxLayout *m_layout = nullptr;
    std::function<void()> m_onClick;
    bool m_hover = false;
};

} // namespace

StatusBar::StatusBar(QWidget *parent)
    : QStatusBar(parent)
{
    setObjectName(QStringLiteral("statusBar"));
    setSizeGripEnabled(false);
    setFixedHeight(22);
    buildItems();
}

void StatusBar::buildItems()
{
    const Theme &theme = Theme::active();
    const QColor soft = theme.color(QStringLiteral("text-soft"));
    const QColor muted = theme.color(QStringLiteral("muted"));

    // ----- Left group -----
    auto *remote = new StatusItem(this);
    remote->addIcon(QStringLiteral("remote"), soft);
    addWidget(remote);

    auto *branch = new StatusItem(this);
    branch->addIcon(QStringLiteral("branch"), soft);
    branch->addText(QStringLiteral("main"), soft);
    branch->addIcon(QStringLiteral("sync"), soft, 11);
    branch->addText(QStringLiteral("2↑ 0↓"), muted);
    addWidget(branch);

    auto *problems = new StatusItem(this);
    problems->addIcon(QStringLiteral("err"), theme.color(QStringLiteral("err")));
    m_errorCountLabel = problems->addText(QStringLiteral("0"), soft);
    problems->addIcon(QStringLiteral("warn"), theme.color(QStringLiteral("warn")));
    m_warningCountLabel = problems->addText(QStringLiteral("0"), soft);
    problems->setOnClick([this]() { emit panelToggleRequested(); });
    addWidget(problems);

    auto *output = new StatusItem(this);
    output->addIcon(QStringLiteral("output"), soft);
    output->addText(QStringLiteral("clangd"), soft);
    addWidget(output);

    auto *task = new StatusItem(this);
    task->addIcon(QStringLiteral("sync"), soft);
    task->addText(QStringLiteral("indexing · 47/87"), muted);
    addWidget(task);

    auto *target = new StatusItem(this);
    target->addIcon(QStringLiteral("beaker"), soft);
    target->addText(QStringLiteral("MalloyWriter (Release)"), soft);
    addWidget(target);

    // ----- Right group -----
    auto *cursor = new StatusItem(this);
    m_cursorLabel = cursor->addText(QStringLiteral("Ln 1, Col 1"), soft);
    addPermanentWidget(cursor);

    auto *indent = new StatusItem(this);
    indent->addText(QStringLiteral("Spaces: 4"), soft);
    addPermanentWidget(indent);

    auto *encoding = new StatusItem(this);
    encoding->addText(QStringLiteral("UTF-8"), soft);
    addPermanentWidget(encoding);

    auto *eol = new StatusItem(this);
    eol->addText(QStringLiteral("LF"), soft);
    addPermanentWidget(eol);

    auto *language = new StatusItem(this);
    m_languageIcon = language->addIcon(QStringLiteral("filecpp"), theme.color(QStringLiteral("tk-type")), 11);
    m_languageLabel = language->addText(QStringLiteral("Plain Text"), soft);
    addPermanentWidget(language);

    auto *clangd = new StatusItem(this);
    m_clangdDot = clangd->addDot(theme.color(QStringLiteral("faint")));
    m_clangdLabel = clangd->addText(QStringLiteral("clangd"), soft);
    addPermanentWidget(clangd);

    auto *assist = new StatusItem(this);
    assist->addIcon(QStringLiteral("ai"), theme.color(QStringLiteral("accent")), 11);
    assist->addText(QStringLiteral("local"), soft);
    addPermanentWidget(assist);

    auto *bell = new StatusItem(this);
    bell->addIcon(QStringLiteral("bell"), soft, 11);
    bell->addText(QStringLiteral("2"), soft);
    addPermanentWidget(bell);

    auto *palette = new StatusItem(this);
    palette->addIcon(QStringLiteral("cmdK"), soft, 11);
    palette->setOnClick([this]() { emit commandPaletteRequested(); });
    addPermanentWidget(palette);
}

void StatusBar::setCursorPosition(int line, int column)
{
    if (m_cursorLabel) {
        m_cursorLabel->setText(tr("Ln %1, Col %2").arg(line).arg(column));
    }
}

void StatusBar::setLanguageMode(const QString &id, const QString &iconName, const QColor &color)
{
    if (m_languageLabel) {
        m_languageLabel->setText(id);
    }
    if (m_languageIcon) {
        m_languageIcon->setPixmap(Icon::pixmap(iconName, 11, color));
    }
}

void StatusBar::setProblemCounts(int errors, int warnings)
{
    if (m_errorCountLabel) {
        m_errorCountLabel->setText(QString::number(errors));
    }
    if (m_warningCountLabel) {
        m_warningCountLabel->setText(QString::number(warnings));
    }
}

void StatusBar::setClangdStatus(const QString &text, bool ok)
{
    if (m_clangdLabel) {
        m_clangdLabel->setText(text);
    }
    if (m_clangdDot) {
        const Theme &theme = Theme::active();
        m_clangdDot->setPixmap(dotPixmap(theme.color(ok ? QStringLiteral("ok") : QStringLiteral("err"))));
    }
}

} // namespace MalloyWriter::Workbench

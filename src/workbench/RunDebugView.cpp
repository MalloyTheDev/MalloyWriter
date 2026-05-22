#include "workbench/RunDebugView.hpp"

#include "base/Theme.hpp"
#include "workbench/Icon.hpp"

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QToolButton>
#include <QVBoxLayout>

namespace MalloyWriter::Workbench {

namespace {

using Base::Theme;

QLabel *sectionHeader(const QString &title, QWidget *parent)
{
    auto *label = new QLabel(title, parent);
    label->setObjectName(QStringLiteral("groupHeader"));
    label->setContentsMargins(14, 10, 8, 4);
    return label;
}

QLabel *mutedRow(const QString &text, QWidget *parent)
{
    auto *label = new QLabel(text, parent);
    label->setContentsMargins(18, 0, 8, 2);
    QPalette pal = label->palette();
    pal.setColor(QPalette::WindowText, Theme::active().color(QStringLiteral("muted")));
    label->setPalette(pal);
    return label;
}

} // namespace

RunDebugView::RunDebugView(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 8, 0, 8);
    layout->setSpacing(0);

    // Launch row: run button + target selector + configure.
    auto *launchRow = new QWidget(this);
    auto *launchLayout = new QHBoxLayout(launchRow);
    launchLayout->setContentsMargins(12, 0, 8, 6);
    launchLayout->setSpacing(6);

    auto *runButton = new QToolButton(launchRow);
    runButton->setObjectName(QStringLiteral("iconBtn"));
    runButton->setIcon(Icon::icon(QStringLiteral("play"), 14, Theme::active().color(QStringLiteral("ok"))));
    runButton->setIconSize(QSize(14, 14));
    runButton->setToolTip(tr("Start Debugging"));
    runButton->setCursor(Qt::PointingHandCursor);
    launchLayout->addWidget(runButton);

    m_targets = new QComboBox(launchRow);
    m_targets->addItem(tr("No configured targets"));
    launchLayout->addWidget(m_targets, 1);

    auto *configureButton = new QToolButton(launchRow);
    configureButton->setObjectName(QStringLiteral("iconBtn"));
    configureButton->setIcon(Icon::icon(QStringLiteral("settings"), 13, Theme::active().color(QStringLiteral("text-soft"))));
    configureButton->setIconSize(QSize(13, 13));
    configureButton->setToolTip(tr("Configure launch"));
    launchLayout->addWidget(configureButton);

    layout->addWidget(launchRow);

    // Debug sections (static until the DAP phase).
    layout->addWidget(sectionHeader(tr("VARIABLES"), this));
    layout->addWidget(mutedRow(tr("Not running"), this));
    layout->addWidget(sectionHeader(tr("WATCH"), this));
    layout->addWidget(mutedRow(tr("Add expression…"), this));
    layout->addWidget(sectionHeader(tr("CALL STACK"), this));
    layout->addWidget(mutedRow(tr("Not running"), this));
    layout->addWidget(sectionHeader(tr("BREAKPOINTS"), this));
    layout->addWidget(mutedRow(tr("No breakpoints"), this));
    layout->addStretch(1);

    connect(runButton, &QToolButton::clicked, this, [this]() {
        emit runRequested(m_targets->currentText());
    });
}

void RunDebugView::setTargets(const QStringList &targets)
{
    m_targets->clear();
    if (targets.isEmpty()) {
        m_targets->addItem(tr("No configured targets"));
        return;
    }
    m_targets->addItems(targets);
}

} // namespace MalloyWriter::Workbench

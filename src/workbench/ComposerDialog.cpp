#include "workbench/ComposerDialog.hpp"

#include "base/Theme.hpp"
#include "workbench/Icon.hpp"

#include <QButtonGroup>
#include <QHBoxLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>

namespace MalloyWriter::Workbench {

ComposerDialog::ComposerDialog(QWidget *parent)
    : QDialog(parent)
{
    setObjectName(QStringLiteral("composer"));
    setWindowTitle(tr("MW Assist"));
    setModal(true);
    resize(580, 340);

    using Base::Theme;
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 14, 16, 14);
    layout->setSpacing(10);

    // Header: title + mode toggle.
    auto *header = new QHBoxLayout;
    auto *icon = new QLabel(this);
    icon->setPixmap(Icon::pixmap(QStringLiteral("ai"), 14, Theme::active().color(QStringLiteral("accent"))));
    header->addWidget(icon);
    auto *title = new QLabel(tr("MW Assist"), this);
    QFont titleFont = title->font();
    titleFont.setWeight(QFont::DemiBold);
    title->setFont(titleFont);
    header->addWidget(title);
    header->addStretch(1);

    auto *modeGroup = new QButtonGroup(this);
    modeGroup->setExclusive(true);
    for (const QString &mode : {tr("Ask"), tr("Edit"), tr("Agent")}) {
        auto *button = new QPushButton(mode, this);
        button->setCheckable(true);
        button->setCursor(Qt::PointingHandCursor);
        modeGroup->addButton(button);
        header->addWidget(button);
    }
    if (!modeGroup->buttons().isEmpty()) {
        modeGroup->buttons().constLast()->setChecked(true); // Agent
    }
    layout->addLayout(header);

    // Prompt.
    auto *prompt = new QPlainTextEdit(this);
    prompt->setPlaceholderText(tr("Describe the change you want — or ask a question about the workspace…"));
    layout->addWidget(prompt, 1);

    // Context chips.
    auto *chips = new QHBoxLayout;
    chips->setSpacing(6);
    auto *chipLabel = new QLabel(tr("Context:"), this);
    QPalette chipPal = chipLabel->palette();
    chipPal.setColor(QPalette::WindowText, Theme::active().color(QStringLiteral("muted")));
    chipLabel->setPalette(chipPal);
    chips->addWidget(chipLabel);
    for (const QString &chip : {tr("active file"), tr("selection")}) {
        auto *button = new QPushButton(chip, this);
        button->setEnabled(false);
        chips->addWidget(button);
    }
    chips->addStretch(1);
    layout->addLayout(chips);

    // Quick prompts.
    auto *quick = new QHBoxLayout;
    quick->setSpacing(6);
    for (const QString &q : {tr("Explain selection"), tr("Refactor"), tr("Generate tests")}) {
        auto *button = new QPushButton(q, this);
        button->setEnabled(false);
        quick->addWidget(button);
    }
    quick->addStretch(1);
    layout->addLayout(quick);

    // Footer: model + send.
    auto *footer = new QHBoxLayout;
    auto *model = new QLabel(tr("qwen2.5-coder-14b · local · disabled by default"), this);
    QPalette modelPal = model->palette();
    modelPal.setColor(QPalette::WindowText, Theme::active().color(QStringLiteral("muted")));
    model->setPalette(modelPal);
    footer->addWidget(model, 1);
    auto *send = new QPushButton(tr("Send"), this);
    send->setEnabled(false);
    send->setToolTip(tr("Enable a provider in Settings → MW Assist"));
    footer->addWidget(send);
    auto *close = new QPushButton(tr("Close"), this);
    connect(close, &QPushButton::clicked, this, &QDialog::reject);
    footer->addWidget(close);
    layout->addLayout(footer);
}

} // namespace MalloyWriter::Workbench

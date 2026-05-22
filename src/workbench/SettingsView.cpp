#include "workbench/SettingsView.hpp"

#include "base/Color.hpp"
#include "base/Theme.hpp"
#include "workbench/ThemeApplication.hpp"

#include <QButtonGroup>
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QStackedWidget>
#include <QVBoxLayout>

namespace MalloyWriter::Workbench {

namespace {

using Base::Theme;
using Base::ThemeVariant;

QLabel *h1(const QString &text, QWidget *parent)
{
    auto *label = new QLabel(text, parent);
    QFont font = label->font();
    font.setPixelSize(20);
    font.setWeight(QFont::DemiBold);
    label->setFont(font);
    return label;
}

QLabel *subtitle(const QString &text, QWidget *parent)
{
    auto *label = new QLabel(text, parent);
    QPalette pal = label->palette();
    pal.setColor(QPalette::WindowText, Theme::active().color(QStringLiteral("muted")));
    label->setPalette(pal);
    label->setContentsMargins(0, 0, 0, 8);
    return label;
}

QLabel *rowLabel(const QString &text, QWidget *parent)
{
    auto *label = new QLabel(text, parent);
    label->setMinimumWidth(160);
    return label;
}

} // namespace

SettingsView::SettingsView(QWidget *parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("settingsView"));
    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    auto *nav = new QListWidget(this);
    nav->setFixedWidth(190);
    nav->addItems({tr("Appearance"), tr("Editor"), tr("Keyboard"), tr("MW Assist")});
    nav->setCurrentRow(0);
    layout->addWidget(nav);

    m_content = new QStackedWidget(this);
    m_content->addWidget(buildAppearancePage());
    m_content->addWidget(buildEditorPage());
    m_content->addWidget(buildKeyboardPage());
    m_content->addWidget(buildAssistPage());
    layout->addWidget(m_content, 1);

    connect(nav, &QListWidget::currentRowChanged, m_content, &QStackedWidget::setCurrentIndex);
}

void SettingsView::applyThemeChange()
{
    applyTheme(Theme::active());
}

QWidget *SettingsView::buildAppearancePage()
{
    auto *page = new QWidget(this);
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 28, 32, 28);
    layout->setSpacing(6);

    layout->addWidget(h1(tr("Appearance"), page));
    layout->addWidget(subtitle(tr("Color theme, accent, and layout. Changes apply immediately."), page));

    // Theme variant (functional).
    layout->addSpacing(8);
    layout->addWidget(rowLabel(tr("Color theme"), page));
    auto *themeRow = new QHBoxLayout;
    themeRow->setSpacing(8);
    auto *themeGroup = new QButtonGroup(page);
    themeGroup->setExclusive(true);
    const struct { const char *label; ThemeVariant variant; } variants[] = {
        {"Slate", ThemeVariant::Slate}, {"Midnight", ThemeVariant::Midnight}, {"Carbon", ThemeVariant::Carbon},
    };
    for (const auto &entry : variants) {
        auto *button = new QPushButton(QString::fromLatin1(entry.label), page);
        button->setCheckable(true);
        button->setCursor(Qt::PointingHandCursor);
        button->setChecked(Theme::active().variant() == entry.variant);
        themeGroup->addButton(button);
        themeRow->addWidget(button);
        const ThemeVariant variant = entry.variant;
        connect(button, &QPushButton::clicked, this, [this, variant]() {
            Theme::active().setVariant(variant);
            applyThemeChange();
        });
    }
    themeRow->addStretch(1);
    layout->addLayout(themeRow);

    // Accent hue (functional).
    layout->addSpacing(16);
    layout->addWidget(rowLabel(tr("Accent color"), page));
    auto *accentRow = new QHBoxLayout;
    accentRow->setSpacing(8);
    const struct { const char *name; double hue; } accents[] = {
        {"Teal", 215}, {"Mint", 145}, {"Amber", 30}, {"Indigo", 280}, {"Magenta", 325},
    };
    for (const auto &accent : accents) {
        auto *swatch = new QPushButton(page);
        swatch->setFixedSize(28, 28);
        swatch->setCursor(Qt::PointingHandCursor);
        swatch->setToolTip(QString::fromLatin1(accent.name));
        const QColor color = Base::oklchToColor({0.74, 0.13, accent.hue});
        swatch->setStyleSheet(QStringLiteral("background-color: %1; border: 2px solid #3a3f47; border-radius: 14px;").arg(color.name()));
        const double hue = accent.hue;
        connect(swatch, &QPushButton::clicked, this, [this, hue]() {
            Theme::active().setAccentHue(hue);
            applyThemeChange();
        });
        accentRow->addWidget(swatch);
    }
    accentRow->addStretch(1);
    layout->addLayout(accentRow);

    layout->addStretch(1);
    return page;
}

QWidget *SettingsView::buildEditorPage()
{
    auto *page = new QWidget(this);
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 28, 32, 28);
    layout->setSpacing(10);
    layout->addWidget(h1(tr("Editor"), page));
    layout->addWidget(subtitle(tr("Font, indentation, and behavior."), page));

    auto addRow = [&](const QString &label, QWidget *control) {
        auto *row = new QHBoxLayout;
        row->addWidget(rowLabel(label, page));
        row->addWidget(control);
        row->addStretch(1);
        layout->addLayout(row);
    };
    auto *family = new QComboBox(page);
    family->addItems({tr("Geist Mono"), tr("Cascadia Mono"), tr("Consolas")});
    addRow(tr("Font family"), family);
    auto *wrap = new QComboBox(page);
    wrap->addItems({tr("Off"), tr("On"), tr("Bounded")});
    addRow(tr("Word wrap"), wrap);
    auto *tab = new QComboBox(page);
    tab->addItems({"2", "4", "8"});
    tab->setCurrentText("4");
    addRow(tr("Tab size"), tab);
    layout->addStretch(1);
    return page;
}

QWidget *SettingsView::buildKeyboardPage()
{
    auto *page = new QWidget(this);
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 28, 32, 28);
    layout->setSpacing(6);
    layout->addWidget(h1(tr("Keyboard Shortcuts"), page));
    layout->addWidget(subtitle(tr("Common bindings (editing arrives with the keymap phase)."), page));

    const struct { const char *command; const char *keys; } rows[] = {
        {"Command Palette", "Ctrl+Shift+P"}, {"Go to File", "Ctrl+P"}, {"Find", "Ctrl+F"},
        {"Save", "Ctrl+S"}, {"Toggle Panel", "Ctrl+J"}, {"Build", "Ctrl+Shift+B"},
        {"Settings", "Ctrl+,"},
    };
    for (const auto &row : rows) {
        auto *line = new QHBoxLayout;
        line->addWidget(rowLabel(QString::fromLatin1(row.command), page));
        auto *keys = new QLabel(QString::fromLatin1(row.keys), page);
        QPalette pal = keys->palette();
        pal.setColor(QPalette::WindowText, Theme::active().color(QStringLiteral("text-soft")));
        keys->setPalette(pal);
        line->addWidget(keys);
        line->addStretch(1);
        layout->addLayout(line);
    }
    layout->addStretch(1);
    return page;
}

QWidget *SettingsView::buildAssistPage()
{
    auto *page = new QWidget(this);
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(32, 28, 32, 28);
    layout->setSpacing(10);
    layout->addWidget(h1(tr("MW Assist"), page));
    layout->addWidget(subtitle(tr("Disabled by default. Local providers run on this machine; cloud providers need a key."), page));

    auto *provider = new QComboBox(page);
    provider->addItems({tr("Off"), tr("Local · qwen2.5-coder-14b"), tr("Local · llama-3.1-8b"),
                        tr("Anthropic · Claude"), tr("OpenAI · gpt-4.1")});
    auto *providerRow = new QHBoxLayout;
    providerRow->addWidget(rowLabel(tr("Provider"), page));
    providerRow->addWidget(provider);
    providerRow->addStretch(1);
    layout->addLayout(providerRow);
    layout->addStretch(1);
    return page;
}

} // namespace MalloyWriter::Workbench

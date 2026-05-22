#include "base/Theme.hpp"

namespace MalloyWriter::Base {

namespace {

// Base (Slate) tokens, transcribed from MalloyIDE/styles.css :root.
QHash<QString, Oklch> slateTokens()
{
    return {
        // Surfaces
        {"bg",            {0.205, 0.005, 245}},
        {"bg-elev",       {0.225, 0.005, 245}},
        {"bg-titlebar",   {0.18,  0.005, 245}},
        {"bg-activity",   {0.165, 0.005, 245}},
        {"bg-statusbar",  {0.205, 0.005, 245}},
        {"surface",       {0.255, 0.006, 245}},
        {"surface-hi",    {0.30,  0.006, 245}},
        {"border",        {0.32,  0.005, 245}},
        {"border-soft",   {0.265, 0.005, 245}},
        // Text
        {"text",          {0.92,  0.008, 245}},
        {"text-soft",     {0.76,  0.006, 245}},
        {"muted",         {0.60,  0.006, 245}},
        {"faint",         {0.46,  0.006, 245}},
        // Accent foreground (text drawn on accent fills); accent hues are applied later.
        {"accent-fg",     {0.14,  0.02,  245}},
        // AI / secondary accent (violet)
        {"ai",            {0.78,  0.16,  295}},
        {"ai-deep",       {0.58,  0.16,  295}},
        {"ai-bg",         {0.30,  0.07,  295}},
        // Brand gradient stops
        {"grad-a",        {0.84,  0.16,  200}},
        {"grad-b",        {0.65,  0.18,  280}},
        // Semantic
        {"ok",            {0.74,  0.13,  150}},
        {"warn",          {0.78,  0.14,  80}},
        {"err",           {0.68,  0.18,  25}},
        {"info",          {0.74,  0.12,  230}},
        {"mod",           {0.78,  0.13,  55}},
        // Syntax
        {"tk-type",       {0.82,  0.10,  175}},
        {"tk-string",     {0.78,  0.13,  50}},
        {"tk-num",        {0.85,  0.13,  110}},
        {"tk-comment",    {0.55,  0.04,  130}},
        {"tk-fn",         {0.83,  0.12,  75}},
        {"tk-var",        {0.88,  0.005, 245}},
        {"tk-punct",      {0.70,  0.006, 245}},
        {"tk-pp",         {0.75,  0.12,  295}},
        {"tk-prop",       {0.85,  0.11,  195}},
        {"tk-ctrl",       {0.78,  0.13,  320}},
    };
}

// Surface/border overrides per variant (other tokens are shared with Slate).
void applyVariantOverrides(QHash<QString, Oklch> &tokens, ThemeVariant variant)
{
    if (variant == ThemeVariant::Midnight) {
        tokens["bg"]           = {0.16,  0.008, 260};
        tokens["bg-elev"]      = {0.19,  0.008, 260};
        tokens["bg-titlebar"]  = {0.14,  0.008, 260};
        tokens["bg-activity"]  = {0.13,  0.008, 260};
        tokens["bg-statusbar"] = {0.16,  0.008, 260};
        tokens["surface"]      = {0.22,  0.009, 260};
        tokens["surface-hi"]   = {0.27,  0.009, 260};
        tokens["border"]       = {0.28,  0.009, 260};
        tokens["border-soft"]  = {0.23,  0.008, 260};
    } else if (variant == ThemeVariant::Carbon) {
        tokens["bg"]           = {0.18,  0.0, 0};
        tokens["bg-elev"]      = {0.20,  0.0, 0};
        tokens["bg-titlebar"]  = {0.155, 0.0, 0};
        tokens["bg-activity"]  = {0.14,  0.0, 0};
        tokens["bg-statusbar"] = {0.18,  0.0, 0};
        tokens["surface"]      = {0.24,  0.0, 0};
        tokens["surface-hi"]   = {0.29,  0.0, 0};
        tokens["border"]       = {0.30,  0.0, 0};
        tokens["border-soft"]  = {0.25,  0.0, 0};
    }
}

} // namespace

QHash<QString, Oklch> Theme::resolveTokens() const
{
    QHash<QString, Oklch> tokens = slateTokens();
    applyVariantOverrides(tokens, m_variant);

    // Accent tokens are parameterized by hue (see MalloyIDE/app.jsx setTweak).
    tokens["accent"]      = {0.74, 0.13, m_accentHue};
    tokens["accent-deep"] = {0.55, 0.13, m_accentHue};
    tokens["accent-bg"]   = {0.30, 0.06, m_accentHue};
    tokens["tk-kw"]       = {0.74, 0.12, m_accentHue};

    return tokens;
}

const QHash<QString, QColor> &Theme::resolvedPalette() const
{
    if (!m_paletteValid) {
        m_paletteCache.clear();
        const auto tokens = resolveTokens();
        for (auto it = tokens.constBegin(); it != tokens.constEnd(); ++it) {
            m_paletteCache.insert(it.key(), oklchToColor(it.value()));
        }
        m_paletteValid = true;
    }
    return m_paletteCache;
}

QColor Theme::color(const QString &token) const
{
    return resolvedPalette().value(token);
}

QHash<QString, QColor> Theme::palette() const
{
    return resolvedPalette();
}

QString Theme::variantId(ThemeVariant variant)
{
    switch (variant) {
    case ThemeVariant::Midnight: return QStringLiteral("midnight");
    case ThemeVariant::Carbon:   return QStringLiteral("carbon");
    case ThemeVariant::Slate:    break;
    }
    return QStringLiteral("slate");
}

ThemeVariant Theme::variantFromId(const QString &id)
{
    if (id == QStringLiteral("midnight")) return ThemeVariant::Midnight;
    if (id == QStringLiteral("carbon"))   return ThemeVariant::Carbon;
    return ThemeVariant::Slate;
}

Theme &Theme::active()
{
    static Theme instance;
    return instance;
}

QString Theme::styleSheet() const
{
    const auto resolved = palette();

    // Template uses %token% placeholders, substituted from the palette below.
    QString sheet = QStringLiteral(R"QSS(
QWidget {
    color: %text%;
    font-family: "Geist", "Segoe UI", sans-serif;
    font-size: 13px;
}
QMainWindow, QDialog { background-color: %bg%; }
QMainWindow::separator { background-color: %border-soft%; width: 1px; height: 1px; }

/* Workbench shell components (by objectName) */
#activityBar { background-color: %bg-activity%; border-right: 1px solid %border-soft%; }
#sidebar { background-color: %bg-elev%; border-right: 1px solid %border-soft%; }
#sidebarHeader { background-color: %bg-elev%; border-bottom: 1px solid %border-soft%; }
#sidebarBody { background-color: %bg-elev%; }
#editorRegion { background-color: %bg%; }
#sidebarTitle { color: %text-soft%; font-size: 11px; font-weight: 600; }
#placeholderText { color: %muted%; font-size: 13px; }
#groupHeader { color: %text-soft%; font-size: 11px; font-weight: 600; }
QPushButton#openEditorRow {
    background: transparent; border: 0; border-radius: 0;
    padding: 3px 8px 3px 14px; text-align: left; color: %text-soft%;
}
QPushButton#openEditorRow:hover { background-color: %surface%; }
QPushButton#openEditorRow[active="true"] { color: %text%; background-color: %accent-bg%; }
QToolTip {
    background-color: %surface%; color: %text%;
    border: 1px solid %border%; padding: 4px 6px;
}

QMenuBar { background-color: %bg-titlebar%; color: %text-soft%; padding: 2px 4px; }
QMenuBar::item { background: transparent; padding: 4px 10px; border-radius: 4px; }
QMenuBar::item:selected { background-color: %surface%; color: %text%; }
QMenuBar::item:pressed { background-color: %surface-hi%; color: %text%; }

QMenu { background-color: %bg-elev%; color: %text-soft%; border: 1px solid %border%; padding: 4px; }
QMenu::item { padding: 5px 24px 5px 12px; border-radius: 4px; }
QMenu::item:selected { background-color: %accent-bg%; color: %text%; }
QMenu::item:disabled { color: %faint%; }
QMenu::separator { height: 1px; background-color: %border-soft%; margin: 4px 8px; }

QToolBar { background-color: %bg-titlebar%; border: 0; spacing: 2px; padding: 3px; }
QToolBar QToolButton { background: transparent; color: %text-soft%; padding: 4px 8px; border-radius: 4px; }
QToolBar QToolButton:hover { background-color: %surface%; color: %text%; }
QToolBar QToolButton:pressed { background-color: %surface-hi%; }

QDockWidget { color: %text-soft%; }
QDockWidget::title {
    background-color: %bg-elev%; padding: 6px 10px;
    border-bottom: 1px solid %border-soft%;
}

QTreeView, QListView, QTableView {
    background-color: %bg-elev%; border: 0; outline: 0;
    selection-background-color: %accent-bg%; selection-color: %text%;
    alternate-background-color: %bg-elev%;
}
QTreeView::item, QListView::item { padding: 3px 2px; border: 0; }
QTreeView::item:hover, QListView::item:hover { background-color: %surface%; }
QTreeView::item:selected, QListView::item:selected { background-color: %accent-bg%; color: %text%; }
QHeaderView::section {
    background-color: %bg-elev%; color: %muted%;
    border: 0; border-bottom: 1px solid %border-soft%; padding: 4px 8px;
}

QTabWidget::pane { border: 0; border-top: 1px solid %border-soft%; background-color: %bg%; }
QTabBar { background-color: %bg-titlebar%; }
QTabBar::tab {
    background-color: %bg-titlebar%; color: %muted%;
    padding: 7px 14px; border: 0; border-right: 1px solid %border-soft%;
}
QTabBar::tab:selected { background-color: %bg%; color: %text%; border-top: 2px solid %accent%; }
QTabBar::tab:hover:!selected { color: %text-soft%; }

QPlainTextEdit, QTextEdit {
    background-color: %bg%; color: %text%; border: 0;
    selection-background-color: %accent-bg%; selection-color: %text%;
    font-family: "Geist Mono", "Consolas", monospace; font-size: 13px;
}

QStatusBar { background-color: %bg-statusbar%; color: %text-soft%; border-top: 1px solid %border-soft%; }
QStatusBar::item { border: 0; }
QStatusBar QLabel { color: %text-soft%; }

QLineEdit, QSpinBox, QComboBox {
    background-color: %bg%; color: %text%;
    border: 1px solid %border%; border-radius: 4px; padding: 4px 8px;
    selection-background-color: %accent-bg%;
}
QLineEdit:focus, QSpinBox:focus, QComboBox:focus { border: 1px solid %accent%; }
QComboBox::drop-down { border: 0; width: 18px; }
QComboBox QAbstractItemView {
    background-color: %bg-elev%; border: 1px solid %border%;
    selection-background-color: %accent-bg%; selection-color: %text%;
}

QPushButton {
    background-color: %surface%; color: %text%;
    border: 1px solid %border%; border-radius: 4px; padding: 5px 14px;
}
QPushButton:hover { background-color: %surface-hi%; }
QPushButton:pressed { background-color: %accent-bg%; }
QPushButton:default { background-color: %accent-deep%; color: %accent-fg%; border: 0; }
QPushButton:checked { background-color: %accent-bg%; color: %text%; border: 1px solid %accent%; }
QPushButton:disabled { color: %faint%; border-color: %border-soft%; }

QToolButton { background: transparent; color: %text-soft%; border: 0; border-radius: 3px; padding: 3px; }
QToolButton:hover { background-color: %surface%; color: %text%; }
QToolButton:checked { color: %accent%; background-color: %accent-bg%; }

#bottomPanel { background-color: %bg%; border-top: 1px solid %border-soft%; }
#panelTabBar { background-color: %bg-titlebar%; border-bottom: 1px solid %border-soft%; }
QPushButton#panelTab {
    background: transparent; border: 0; border-top: 2px solid transparent;
    border-radius: 0; padding: 5px 12px; color: %muted%;
}
QPushButton#panelTab:hover { color: %text-soft%; }
QPushButton#panelTab:checked { color: %text%; border-top: 2px solid %accent%; }

#startCard { background-color: %bg-elev%; border: 1px solid %border-soft%; border-radius: 6px; }
#startCard:hover { border-color: %accent%; }
#welcomeTitle { color: %text%; }
#recentRow { color: %text-soft%; border-radius: 4px; padding: 4px 6px; text-align: left; background: transparent; border: 0; }
#recentRow:hover { background-color: %surface%; }

QScrollBar:vertical { background: transparent; width: 12px; margin: 0; }
QScrollBar::handle:vertical { background-color: %border%; border-radius: 6px; min-height: 24px; }
QScrollBar::handle:vertical:hover { background-color: %muted%; }
QScrollBar:horizontal { background: transparent; height: 12px; margin: 0; }
QScrollBar::handle:horizontal { background-color: %border%; border-radius: 6px; min-width: 24px; }
QScrollBar::handle:horizontal:hover { background-color: %muted%; }
QScrollBar::add-line, QScrollBar::sub-line { width: 0; height: 0; background: none; border: 0; }
QScrollBar::add-page, QScrollBar::sub-page { background: none; }
)QSS");

    // Substitute every %token% with its resolved hex value.
    for (auto it = resolved.constBegin(); it != resolved.constEnd(); ++it) {
        sheet.replace(QStringLiteral("%%1%").arg(it.key()), it.value().name(QColor::HexRgb));
    }
    return sheet;
}

} // namespace MalloyWriter::Base

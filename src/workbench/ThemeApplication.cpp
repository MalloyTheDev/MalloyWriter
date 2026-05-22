#include "workbench/ThemeApplication.hpp"

#include "base/Theme.hpp"

#include <QApplication>
#include <QPalette>
#include <QStyle>
#include <QStyleFactory>

namespace MalloyWriter::Workbench {

namespace {

// Build a QPalette from the theme tokens so native-drawn elements (dialogs,
// tooltips, selections) match the QSS.
QPalette paletteFromTheme(const Base::Theme &theme)
{
    auto c = [&theme](const char *token) { return theme.color(QString::fromLatin1(token)); };

    QPalette palette;
    palette.setColor(QPalette::Window, c("bg"));
    palette.setColor(QPalette::WindowText, c("text"));
    palette.setColor(QPalette::Base, c("bg"));
    palette.setColor(QPalette::AlternateBase, c("bg-elev"));
    palette.setColor(QPalette::Text, c("text"));
    palette.setColor(QPalette::Button, c("surface"));
    palette.setColor(QPalette::ButtonText, c("text"));
    palette.setColor(QPalette::BrightText, c("text"));
    palette.setColor(QPalette::ToolTipBase, c("surface"));
    palette.setColor(QPalette::ToolTipText, c("text"));
    palette.setColor(QPalette::PlaceholderText, c("muted"));
    palette.setColor(QPalette::Highlight, c("accent-bg"));
    palette.setColor(QPalette::HighlightedText, c("text"));
    palette.setColor(QPalette::Link, c("accent"));
    palette.setColor(QPalette::LinkVisited, c("ai"));

    palette.setColor(QPalette::Disabled, QPalette::Text, c("faint"));
    palette.setColor(QPalette::Disabled, QPalette::WindowText, c("faint"));
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, c("faint"));
    return palette;
}

} // namespace

void applyTheme(const Base::Theme &theme)
{
    if (!qApp) {
        return;
    }
    // Fusion honors QPalette + QSS consistently; set it once.
    if (!qApp->style() || qApp->style()->name().compare(QLatin1String("fusion"), Qt::CaseInsensitive) != 0) {
        qApp->setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
    }
    qApp->setPalette(paletteFromTheme(theme));
    qApp->setStyleSheet(theme.styleSheet());
}

} // namespace MalloyWriter::Workbench

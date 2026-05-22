#include "base/Log.hpp"
#include "base/Theme.hpp"
#include "workbench/MainWindow.hpp"

#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QFont>
#include <QFontDatabase>
#include <QPalette>
#include <QStyleFactory>

namespace {

// Register any Geist / Geist Mono font files shipped under <appdir>/fonts.
// Missing files are skipped: the style sheet falls back to Segoe UI / Consolas.
void loadApplicationFonts()
{
    QDir fontDir(QCoreApplication::applicationDirPath() + QStringLiteral("/fonts"));
    if (!fontDir.exists()) {
        return;
    }
    const auto files = fontDir.entryList({QStringLiteral("*.ttf"), QStringLiteral("*.otf")}, QDir::Files);
    for (const QString &file : files) {
        QFontDatabase::addApplicationFont(fontDir.filePath(file));
    }
}

// The base UI font: Geist when bundled, otherwise the platform default.
QFont baseUiFont()
{
    const QString family = QFontDatabase::families().contains(QStringLiteral("Geist"))
        ? QStringLiteral("Geist")
        : QStringLiteral("Segoe UI");
    QFont font(family);
    font.setPixelSize(13);
    return font;
}

// Build a QPalette from the theme tokens so native-drawn elements (dialogs,
// tooltips, selections) match the QSS before/around the stylesheet.
QPalette paletteFromTheme(const MalloyWriter::Base::Theme &theme)
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

void applyTheme(QApplication &app, const MalloyWriter::Base::Theme &theme)
{
    // Fusion honors QPalette + QSS consistently across platforms, unlike the
    // native Windows style which ignores many stylesheet properties.
    app.setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
    app.setPalette(paletteFromTheme(theme));
    app.setStyleSheet(theme.styleSheet());
}

} // namespace

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("Malloy");
    QCoreApplication::setApplicationName("MalloyWriter");
    QCoreApplication::setApplicationVersion("0.1.0");

    MalloyWriter::Base::installMessageHandler();

    loadApplicationFonts();
    app.setFont(baseUiFont());

    MalloyWriter::Base::Theme &theme = MalloyWriter::Base::Theme::active();
    applyTheme(app, theme);

    MalloyWriter::Workbench::MainWindow window;
    window.show();

    return app.exec();
}

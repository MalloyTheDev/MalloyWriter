#include "base/Log.hpp"
#include "base/Theme.hpp"
#include "workbench/MainWindow.hpp"
#include "workbench/ThemeApplication.hpp"

#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QFont>
#include <QFontDatabase>

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

    MalloyWriter::Workbench::applyTheme(MalloyWriter::Base::Theme::active());

    MalloyWriter::Workbench::MainWindow window;
    window.show();

    return app.exec();
}

#pragma once

#include <QColor>
#include <QIcon>
#include <QPixmap>
#include <QString>

namespace MalloyWriter::Workbench {

// File-type icon name + accent color, matching the prototype's FMT tables.
struct FileType {
    QString iconName;
    QColor color;
};

// Renders the MalloyWriter line-icon set (ported from MalloyIDE/icons.jsx) as
// tinted pixmaps/icons via QSvgRenderer. Icons are 16x16 stroke art; the color
// is baked in at render time so callers pick the token color per state.
class Icon {
public:
    static bool has(const QString &name);
    static QPixmap pixmap(const QString &name, int size, const QColor &color);
    static QIcon icon(const QString &name, int size, const QColor &color);

    // Map a file name to its themed icon + color (cpp/hpp/md/json/cmake/git/…).
    static FileType fileType(const QString &fileName);
};

} // namespace MalloyWriter::Workbench

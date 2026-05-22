#pragma once

#include <QColor>
#include <QIcon>
#include <QPixmap>
#include <QString>

namespace MalloyWriter::Workbench {

// Renders the MalloyWriter line-icon set (ported from MalloyIDE/icons.jsx) as
// tinted pixmaps/icons via QSvgRenderer. Icons are 16x16 stroke art; the color
// is baked in at render time so callers pick the token color per state.
class Icon {
public:
    static bool has(const QString &name);
    static QPixmap pixmap(const QString &name, int size, const QColor &color);
    static QIcon icon(const QString &name, int size, const QColor &color);
};

} // namespace MalloyWriter::Workbench

#include "workbench/Icon.hpp"

#include <QGuiApplication>
#include <QHash>
#include <QPainter>
#include <QSvgRenderer>

namespace MalloyWriter::Workbench {

namespace {

// Icon bodies, transcribed from MalloyIDE/icons.jsx (16x16 viewBox). The
// className markers .i / .i-fill are rewritten to presentation attributes at
// render time so Qt's SVG renderer (no CSS) draws them correctly.
const QHash<QString, QString> &iconBodies()
{
    static const QHash<QString, QString> bodies = {
        {"explorer",  R"(<g class="i"><path d="M2.5 3.5h4l1.2 1.6h5.8v7.4a1 1 0 0 1-1 1H3.5a1 1 0 0 1-1-1v-9Z"/></g>)"},
        {"search",    R"(<g class="i"><circle cx="7" cy="7" r="4"/><path d="M10 10l3.5 3.5"/></g>)"},
        {"scm",       R"(<g class="i"><circle cx="4" cy="3.5" r="1.2"/><circle cx="4" cy="12.5" r="1.2"/><circle cx="12" cy="6" r="1.2"/><path d="M4 4.7v6.6M4 8a3 3 0 0 0 3-3h4"/></g>)"},
        {"run",       R"(<g class="i"><circle cx="8" cy="8" r="6"/><path d="M7 5.5l3 2.5-3 2.5v-5Z" class="i-fill"/></g>)"},
        {"ext",       R"(<g class="i"><rect x="2.5" y="2.5" width="5" height="5" rx="0.6"/><rect x="8.5" y="8.5" width="5" height="5" rx="0.6"/><rect x="2.5" y="8.5" width="5" height="5" rx="0.6"/><path d="M11 2.5v3M9.5 4h3"/></g>)"},
        {"test",      R"(<g class="i"><path d="M6 2.5h4v4.5l3 5a1 1 0 0 1-.9 1.5H3.9a1 1 0 0 1-.9-1.5l3-5V2.5Z"/><path d="M5.5 9h5"/></g>)"},
        {"bug",       R"(<g class="i"><circle cx="8" cy="9" r="3.5"/><path d="M5 6.5l-2-1M11 6.5l2-1M5 9h-2M11 9h2M5.5 12.5l-2 1.5M10.5 12.5l2 1.5M6.5 6c0-1.5 .7-2.5 1.5-2.5s1.5 1 1.5 2.5"/></g>)"},
        {"remote",    R"(<g class="i"><path d="M2.5 3.5h11v4h-11zM2.5 8.5h11v4h-11z"/><circle cx="4.5" cy="5.5" r="0.4" class="i-fill"/><circle cx="4.5" cy="10.5" r="0.4" class="i-fill"/><path d="M7 5.5h4M7 10.5h4"/></g>)"},
        {"acc",       R"(<g class="i"><circle cx="8" cy="6" r="2.5"/><path d="M3 13.5c.8-2.4 2.8-3.5 5-3.5s4.2 1.1 5 3.5"/></g>)"},
        {"settings",  R"(<g class="i"><circle cx="8" cy="8" r="2"/><path d="M8 1.5v1.5M8 13v1.5M3.5 3.5l1.1 1.1M11.4 11.4l1.1 1.1M1.5 8h1.5M13 8h1.5M3.5 12.5l1.1-1.1M11.4 4.6l1.1-1.1"/></g>)"},
        {"notif",     R"(<g class="i"><path d="M3.5 11.5h9l-1-1.5v-3a3.5 3.5 0 0 0-7 0v3l-1 1.5Z"/><path d="M6.5 13a1.5 1.5 0 0 0 3 0"/></g>)"},
        {"ai",        R"(<g class="i"><path d="M8 2l1.4 3.1L12.5 6.5l-3.1 1.4L8 11l-1.4-3.1L3.5 6.5l3.1-1.4L8 2Z"/><path d="M12.5 11l.7 1.6L14.8 13l-1.6 .7L12.5 15.3l-.7-1.6L10.2 13l1.6-.7L12.5 11Z"/></g>)"},

        {"chevDown",  R"(<g class="i"><path d="M4 6.2l4 4 4-4"/></g>)"},
        {"chevRight", R"(<g class="i"><path d="M6 4l4 4-4 4"/></g>)"},
        {"chevUp",    R"(<g class="i"><path d="M4 10l4-4 4 4"/></g>)"},
        {"chevLeft",  R"(<g class="i"><path d="M10 4l-4 4 4 4"/></g>)"},

        {"plus",      R"(<g class="i"><path d="M8 3.5v9M3.5 8h9"/></g>)"},
        {"x",         R"(<g class="i"><path d="M4 4l8 8M12 4l-8 8"/></g>)"},
        {"more",      R"(<g class="i-fill"><circle cx="3.5" cy="8" r="1"/><circle cx="8" cy="8" r="1"/><circle cx="12.5" cy="8" r="1"/></g>)"},
        {"refresh",   R"(<g class="i"><path d="M3 8a5 5 0 0 1 8.5-3.5L13 6M13 3v3h-3M13 8a5 5 0 0 1-8.5 3.5L3 10M3 13v-3h3"/></g>)"},
        {"filter",    R"(<g class="i"><path d="M2.5 3.5h11l-4 5v4l-3 1.5v-5.5l-4-5Z"/></g>)"},
        {"collapse",  R"(<g class="i"><path d="M3 6l3-3 3 3M3 10l3 3 3-3M11 3v10"/></g>)"},
        {"split",     R"(<g class="i"><rect x="2.5" y="2.5" width="11" height="11" rx="1"/><path d="M8 2.5v11"/></g>)"},
        {"sidebarL",  R"(<g class="i"><rect x="2" y="2.5" width="12" height="11" rx="1"/><path d="M6 2.5v11"/></g>)"},
        {"panelB",    R"(<g class="i"><rect x="2" y="2.5" width="12" height="11" rx="1"/><path d="M2 10h12"/></g>)"},
        {"sidebarR",  R"(<g class="i"><rect x="2" y="2.5" width="12" height="11" rx="1"/><path d="M10 2.5v11"/></g>)"},

        {"folder",    R"(<g class="i"><path d="M2 4.5h4.5L7.5 6h6.5v6.5a1 1 0 0 1-1 1H3a1 1 0 0 1-1-1v-8Z"/></g>)"},
        {"folderOpen",R"(<g class="i"><path d="M2 4.5h4.5L7.5 6h6.5v1.5M2 4.5V12a1.5 1.5 0 0 0 1.5 1.5h10L15 7.5H4.5Z"/></g>)"},
        {"file",      R"(<g class="i"><path d="M4 2.5h5l3 3v8a1 1 0 0 1-1 1H4a1 1 0 0 1-1-1v-10a1 1 0 0 1 1-1Z"/><path d="M9 2.5v3h3"/></g>)"},
        {"filecpp",   R"(<g class="i"><path d="M4 2.5h5l3 3v8a1 1 0 0 1-1 1H4a1 1 0 0 1-1-1v-10a1 1 0 0 1 1-1Z"/><path d="M9 2.5v3h3"/></g>)"},
        {"filehpp",   R"(<g class="i"><path d="M4 2.5h5l3 3v8a1 1 0 0 1-1 1H4a1 1 0 0 1-1-1v-10a1 1 0 0 1 1-1Z"/><path d="M9 2.5v3h3"/></g>)"},
        {"filemd",    R"(<g class="i"><path d="M4 2.5h5l3 3v8a1 1 0 0 1-1 1H4a1 1 0 0 1-1-1v-10a1 1 0 0 1 1-1Z"/><path d="M9 2.5v3h3M5 11V8l1 2 1-2v3M9.5 8.5v2.5M9.5 11l1-1M9.5 11l-1-1"/></g>)"},
        {"filecmake", R"(<g class="i"><path d="M4 2.5h5l3 3v8a1 1 0 0 1-1 1H4a1 1 0 0 1-1-1v-10a1 1 0 0 1 1-1Z"/><path d="M9 2.5v3h3"/></g>)"},
        {"filegit",   R"(<g class="i"><path d="M4 2.5h5l3 3v8a1 1 0 0 1-1 1H4a1 1 0 0 1-1-1v-10a1 1 0 0 1 1-1Z"/><path d="M9 2.5v3h3"/></g>)"},

        {"branch",    R"(<g class="i"><circle cx="4" cy="3.5" r="1.2"/><circle cx="4" cy="12.5" r="1.2"/><circle cx="11" cy="6" r="1.2"/><path d="M4 4.7v6.6M4 7.5a3 3 0 0 0 3 3h.5"/><path d="M4 8a3 3 0 0 1 3-3h2.7"/></g>)"},
        {"sync",      R"(<g class="i"><path d="M3 8a5 5 0 0 1 8.5-3.5L13 6M13 3v3h-3M13 8a5 5 0 0 1-8.5 3.5L3 10M3 13v-3h3"/></g>)"},
        {"warn",      R"(<g class="i"><path d="M8 2.5l6 11H2l6-11Z"/><path d="M8 6.5v3M8 11.2v.4"/></g>)"},
        {"err",       R"(<g class="i"><circle cx="8" cy="8" r="5.5"/><path d="M5.5 5.5l5 5M10.5 5.5l-5 5"/></g>)"},
        {"info",      R"(<g class="i"><circle cx="8" cy="8" r="5.5"/><path d="M8 7v4M8 5v.5"/></g>)"},
        {"check",     R"(<g class="i"><path d="M3 8.5l3 3 7-7"/></g>)"},
        {"play",      R"(<g class="i"><path d="M5 3l8 5-8 5V3Z" class="i-fill"/></g>)"},
        {"stop",      R"(<g class="i-fill"><rect x="4" y="4" width="8" height="8" rx="1"/></g>)"},
        {"reload",    R"(<g class="i"><path d="M3 8a5 5 0 0 1 8.5-3.5L13 6M13 3v3h-3"/></g>)"},
        {"bell",      R"(<g class="i"><path d="M3.5 11.5h9l-1-1.5v-3a3.5 3.5 0 0 0-7 0v3l-1 1.5Z"/><path d="M6.5 13a1.5 1.5 0 0 0 3 0"/></g>)"},
        {"termIcon",  R"(<g class="i"><rect x="2" y="3" width="12" height="10" rx="1"/><path d="M4.5 6.5l2 1.5-2 1.5M8 10h3"/></g>)"},
        {"terminal",  R"(<g class="i"><path d="M3 5l3 3-3 3M7 11h6"/></g>)"},
        {"output",    R"(<g class="i"><rect x="2.5" y="3" width="11" height="10" rx="1"/><path d="M5 6.5h6M5 9h4M5 11h2"/></g>)"},
        {"debug",     R"(<g class="i"><circle cx="8" cy="9" r="3.5"/><path d="M5 6.5l-2-1M11 6.5l2-1M5 9h-2M11 9h2M5.5 12.5l-2 1.5M10.5 12.5l2 1.5"/></g>)"},
        {"ports",     R"(<g class="i"><rect x="2" y="5" width="12" height="6" rx="0.6"/><circle cx="5" cy="8" r="0.6" class="i-fill"/><circle cx="8" cy="8" r="0.6" class="i-fill"/><circle cx="11" cy="8" r="0.6" class="i-fill"/></g>)"},
        {"maximize",  R"(<g class="i"><path d="M3 3v3M3 3h3M13 3h-3M13 3v3M3 13h3M3 13v-3M13 13v-3M13 13h-3"/></g>)"},
        {"minimize",  R"(<g class="i"><path d="M3 8h10"/></g>)"},
        {"window",    R"(<g class="i"><rect x="2.5" y="3" width="11" height="10" rx="0.8"/></g>)"},
        {"closeWin",  R"(<g class="i"><path d="M4 4l8 8M12 4l-8 8"/></g>)"},

        {"cmdK",      R"(<g class="i"><path d="M4 6a2 2 0 1 1 2 2H4V6ZM8 6a2 2 0 1 0-2 2h2V6ZM4 10a2 2 0 1 0 2-2H4v2ZM8 10a2 2 0 1 1-2-2h2v2ZM12 6.5L10 8l2 1.5"/></g>)"},
        {"goTo",      R"(<g class="i"><path d="M2.5 5h11M2.5 8h7M2.5 11h11M12 6.5l2 1.5-2 1.5"/></g>)"},
        {"beaker",    R"(<g class="i"><path d="M6 2.5h4v4.5l3 5a1 1 0 0 1-.9 1.5H3.9a1 1 0 0 1-.9-1.5l3-5V2.5Z"/><path d="M5.5 9h5"/></g>)"},
        {"starFill",  R"(<g class="i-fill"><path d="M8 2l1.8 3.7L14 6.3l-3 2.9.7 4.2L8 11.4l-3.7 1.9L5 9.2 2 6.3l4.2-.6L8 2Z"/></g>)"},
        {"verified",  R"(<g><path class="i" d="M8 1.5l1.8 1.2 2.2-.4.7 2 1.8 1.3-.7 2.1.4 2.1-1.7 1.4-.8 2-2.2-.3L8 14.5l-1.5-1.4-2.2.3-.8-2L1.8 9.9 2.2 7.8 1.5 5.7 3.3 4.4l.7-2 2.2 .4L8 1.5Z"/><path class="i" d="M5.5 8l2 2 3.5-4"/></g>)"},

        {"symbolClass",     R"(<g class="i"><circle cx="8" cy="8" r="5.5"/><path d="M5 8h6M8 5v6"/></g>)"},
        {"symbolMethod",    R"(<g class="i"><path d="M2.5 6L8 3l5.5 3v4L8 13l-5.5-3V6Z"/><path d="M8 8.5L13.5 6M8 8.5L2.5 6M8 8.5V13"/></g>)"},
        {"symbolField",     R"(<g class="i"><path d="M2.5 8L8 4l5.5 4-5.5 4-5.5-4Z"/></g>)"},
        {"symbolNamespace", R"(<g class="i"><path d="M5 3v10M11 3v10M3 5h2M3 11h2M11 5h2M11 11h2"/></g>)"},

        {"back",      R"(<g class="i"><path d="M10 3l-4 5 4 5"/></g>)"},
        {"fwd",       R"(<g class="i"><path d="M6 3l4 5-4 5"/></g>)"},
    };
    return bodies;
}

QString buildSvg(const QString &name, const QColor &color)
{
    const auto it = iconBodies().constFind(name);
    if (it == iconBodies().constEnd()) {
        return {};
    }

    const QString hex = color.name(QColor::HexRgb);
    QString body = it.value();
    body.replace(QStringLiteral(R"(class="i-fill")"),
                 QStringLiteral(R"(fill="%1" stroke="none")").arg(hex));
    body.replace(QStringLiteral(R"(class="i")"),
                 QStringLiteral(R"(fill="none" stroke="%1" stroke-width="1.4" stroke-linecap="round" stroke-linejoin="round")").arg(hex));

    return QStringLiteral(R"(<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 16 16">%1</svg>)").arg(body);
}

} // namespace

bool Icon::has(const QString &name)
{
    return iconBodies().contains(name);
}

QPixmap Icon::pixmap(const QString &name, int size, const QColor &color)
{
    const qreal dpr = qApp ? qApp->devicePixelRatio() : 1.0;

    // Cache rendered pixmaps: re-parsing the SVG on every widget repaint is
    // wasteful, and icons are drawn frequently (tabs, status bar, activity bar).
    static QHash<QString, QPixmap> cache;
    const QString key = QStringLiteral("%1|%2|%3|%4")
                            .arg(name).arg(size).arg(color.name(QColor::HexArgb)).arg(dpr);
    const auto cached = cache.constFind(key);
    if (cached != cache.constEnd()) {
        return cached.value();
    }

    const QString svg = buildSvg(name, color);
    if (svg.isEmpty()) {
        return {};
    }

    QSvgRenderer renderer(svg.toUtf8());
    QPixmap pixmap(QSize(size, size) * dpr);
    pixmap.setDevicePixelRatio(dpr);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);
    renderer.render(&painter, QRectF(0, 0, size, size));

    cache.insert(key, pixmap);
    return pixmap;
}

QIcon Icon::icon(const QString &name, int size, const QColor &color)
{
    return QIcon(pixmap(name, size, color));
}

} // namespace MalloyWriter::Workbench

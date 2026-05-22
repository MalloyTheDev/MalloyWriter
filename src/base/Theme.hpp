#pragma once

#include "base/Color.hpp"

#include <QColor>
#include <QHash>
#include <QString>

namespace MalloyWriter::Base {

// The three workbench color themes from the design prototype.
enum class ThemeVariant {
    Slate,
    Midnight,
    Carbon,
};

// Holds the MalloyWriter design tokens (OKLCH) and produces resolved QColors
// plus a global Qt style sheet. Variant and accent hue can be changed for live
// theme switching; callers re-apply styleSheet() after a change.
class Theme {
public:
    Theme() = default;

    void setVariant(ThemeVariant variant) { m_variant = variant; m_paletteValid = false; }
    void setAccentHue(double hueDegrees) { m_accentHue = hueDegrees; m_paletteValid = false; }
    ThemeVariant variant() const { return m_variant; }
    double accentHue() const { return m_accentHue; }

    // Resolve a token (CSS var name without the leading "--") to a QColor.
    // Unknown tokens return an invalid QColor.
    QColor color(const QString &token) const;

    // All resolved tokens, keyed by name.
    QHash<QString, QColor> palette() const;

    // The global application style sheet built from the current tokens.
    QString styleSheet() const;

    static QString variantId(ThemeVariant variant);
    static ThemeVariant variantFromId(const QString &id);

    // Process-wide active theme, used by widgets that paint with token colors.
    // main() configures and applies this instance at startup.
    static Theme &active();

private:
    QHash<QString, Oklch> resolveTokens() const;
    const QHash<QString, QColor> &resolvedPalette() const;

    ThemeVariant m_variant = ThemeVariant::Slate;
    double m_accentHue = 215.0; // teal, the prototype default

    // Resolved sRGB palette is cached and rebuilt only when variant/accent change.
    mutable QHash<QString, QColor> m_paletteCache;
    mutable bool m_paletteValid = false;
};

} // namespace MalloyWriter::Base

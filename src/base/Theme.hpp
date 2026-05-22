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

    void setVariant(ThemeVariant variant) { m_variant = variant; }
    void setAccentHue(double hueDegrees) { m_accentHue = hueDegrees; }
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

    ThemeVariant m_variant = ThemeVariant::Slate;
    double m_accentHue = 215.0; // teal, the prototype default
};

} // namespace MalloyWriter::Base

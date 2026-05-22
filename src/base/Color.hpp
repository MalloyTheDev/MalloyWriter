#pragma once

#include <QColor>

namespace MalloyWriter::Base {

// A color in the OKLCH space, matching the design tokens in MalloyIDE/styles.css.
//   l: perceptual lightness, 0..1
//   c: chroma, 0..~0.4
//   h: hue, degrees
struct Oklch {
    double l = 0.0;
    double c = 0.0;
    double h = 0.0;
    double alpha = 1.0;
};

// Convert an OKLCH color to an sRGB QColor, clamped to the sRGB gamut.
QColor oklchToColor(const Oklch &color);

} // namespace MalloyWriter::Base

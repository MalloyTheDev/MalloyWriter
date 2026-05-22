#include "base/Color.hpp"

#include <algorithm>
#include <cmath>

namespace MalloyWriter::Base {

namespace {

constexpr double kPi = 3.14159265358979323846;

// Gamma-encode a single linear-light channel into sRGB (0..1).
double linearToSrgb(double channel)
{
    channel = std::clamp(channel, 0.0, 1.0);
    return channel <= 0.0031308
        ? 12.92 * channel
        : 1.055 * std::pow(channel, 1.0 / 2.4) - 0.055;
}

int toByte(double channel)
{
    return static_cast<int>(std::lround(std::clamp(channel, 0.0, 1.0) * 255.0));
}

} // namespace

QColor oklchToColor(const Oklch &color)
{
    // OKLCH -> OKLab (cylindrical to rectangular).
    const double hRad = color.h * kPi / 180.0;
    const double a = color.c * std::cos(hRad);
    const double b = color.c * std::sin(hRad);
    const double l = color.l;

    // OKLab -> linear sRGB (Björn Ottosson's matrices).
    const double lPrime = l + 0.3963377774 * a + 0.2158037573 * b;
    const double mPrime = l - 0.1055613458 * a - 0.0638541728 * b;
    const double sPrime = l - 0.0894841775 * a - 1.2914855480 * b;

    const double lCubed = lPrime * lPrime * lPrime;
    const double mCubed = mPrime * mPrime * mPrime;
    const double sCubed = sPrime * sPrime * sPrime;

    const double rLinear = 4.0767416621 * lCubed - 3.3077115913 * mCubed + 0.2309699292 * sCubed;
    const double gLinear = -1.2684380046 * lCubed + 2.6097574011 * mCubed - 0.3413193965 * sCubed;
    const double bLinear = -0.0041960863 * lCubed - 0.7034186147 * mCubed + 1.7076147010 * sCubed;

    QColor result;
    result.setRgb(toByte(linearToSrgb(rLinear)),
                  toByte(linearToSrgb(gLinear)),
                  toByte(linearToSrgb(bLinear)));
    result.setAlphaF(static_cast<float>(std::clamp(color.alpha, 0.0, 1.0)));
    return result;
}

} // namespace MalloyWriter::Base

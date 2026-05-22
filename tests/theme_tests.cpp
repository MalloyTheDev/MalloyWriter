#include "base/Color.hpp"
#include "base/Theme.hpp"

#include <QtTest/QtTest>

using MalloyWriter::Base::Oklch;
using MalloyWriter::Base::oklchToColor;
using MalloyWriter::Base::Theme;
using MalloyWriter::Base::ThemeVariant;

class ThemeTests : public QObject {
    Q_OBJECT

private slots:
    void convertsBlackAndWhiteExactly()
    {
        QCOMPARE(oklchToColor({0.0, 0.0, 0.0}).name(QColor::HexRgb), QStringLiteral("#000000"));
        QCOMPARE(oklchToColor({1.0, 0.0, 0.0}).name(QColor::HexRgb), QStringLiteral("#ffffff"));
    }

    void achromaticColorsHaveEqualChannels()
    {
        const QColor gray = oklchToColor({0.5, 0.0, 245.0});
        QVERIFY(gray.isValid());
        QCOMPARE(gray.red(), gray.green());
        QCOMPARE(gray.green(), gray.blue());
    }

    void chromaticColorStaysInGamut()
    {
        const QColor accent = oklchToColor({0.74, 0.13, 215.0});
        QVERIFY(accent.isValid());
        // Teal accent: blue/green should dominate red.
        QVERIFY(accent.blue() > accent.red());
        QVERIFY(accent.green() > accent.red());
    }

    void resolvesKnownTokens()
    {
        Theme theme;
        QVERIFY(theme.color(QStringLiteral("bg")).isValid());
        QVERIFY(theme.color(QStringLiteral("accent")).isValid());
        QVERIFY(theme.color(QStringLiteral("tk-string")).isValid());
        QVERIFY(!theme.color(QStringLiteral("does-not-exist")).isValid());
    }

    void variantsChangeSurfaceColors()
    {
        Theme slate;
        Theme carbon;
        carbon.setVariant(ThemeVariant::Carbon);
        QVERIFY(slate.color(QStringLiteral("bg")) != carbon.color(QStringLiteral("bg")));
    }

    void accentHueChangesAccentColor()
    {
        Theme teal;
        Theme amber;
        amber.setAccentHue(30.0);
        QVERIFY(teal.color(QStringLiteral("accent")) != amber.color(QStringLiteral("accent")));
    }

    void styleSheetSubstitutesEveryToken()
    {
        const QString sheet = Theme().styleSheet();
        QVERIFY(!sheet.isEmpty());
        // Every %token% placeholder must be replaced; no stray '%' should remain.
        QVERIFY(!sheet.contains(QLatin1Char('%')));
        QVERIFY(sheet.contains(QStringLiteral("QMenuBar")));
    }

    void variantIdRoundTrips()
    {
        QCOMPARE(Theme::variantFromId(Theme::variantId(ThemeVariant::Midnight)), ThemeVariant::Midnight);
        QCOMPARE(Theme::variantFromId(Theme::variantId(ThemeVariant::Carbon)), ThemeVariant::Carbon);
        QCOMPARE(Theme::variantFromId(QStringLiteral("unknown")), ThemeVariant::Slate);
    }
};

QTEST_MAIN(ThemeTests)

#include "theme_tests.moc"

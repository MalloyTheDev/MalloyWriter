#include "platform/ToolchainKit.hpp"

#include <QtTest/QtTest>

using MalloyWriter::Platform::ToolchainKit;
using MalloyWriter::Platform::ToolchainKitDetector;

class ToolchainKitTests : public QObject {
    Q_OBJECT

private slots:
    void reportsMissingTools()
    {
        ToolchainKit kit;
        kit.id = "broken";
        kit.displayName = "Broken";
        kit.cmakeProgram = "Z:/missing/cmake.exe";
        kit.makeProgram = "Z:/missing/ninja.exe";
        kit.cCompiler = "Z:/missing/gcc.exe";
        kit.cxxCompiler = "Z:/missing/g++.exe";
        kit.clangdProgram = "Z:/missing/clangd.exe";
        kit.debuggerProgram = "Z:/missing/gdb.exe";

        QVERIFY(!kit.isUsable());
        QCOMPARE(kit.missingTools().size(), 6);
    }

    void detectsDefaultKitShape()
    {
        const auto kit = ToolchainKitDetector::defaultWindowsKit();
        QCOMPARE(kit.id, QString("msys2-mingw64"));
        QVERIFY(!kit.displayName.isEmpty());
        QCOMPARE(kit.generator, QString("Ninja"));
    }
};

QTEST_MAIN(ToolchainKitTests)

#include "toolchain_kit_tests.moc"

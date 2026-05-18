#include "platform/BuildSystem.hpp"

#include <QDir>
#include <QtTest/QtTest>

using MalloyWriter::Platform::BuildSettings;
using MalloyWriter::Platform::CMakeBuildPlanner;

class BuildSystemTests : public QObject {
    Q_OBJECT

private slots:
    void createsConfigureCommand()
    {
        BuildSettings settings;
        settings.cmakeProgram = "cmake";
        settings.generator = "Ninja";
        settings.buildDirectory = "out/build";
        settings.makeProgram = "ninja";
        settings.cCompiler = "gcc";
        settings.cxxCompiler = "g++";

        const auto command = CMakeBuildPlanner::configureCommand("E:/Project", settings);
        QCOMPARE(command.program, QString("cmake"));
        QCOMPARE(command.workingDirectory, QDir::cleanPath("E:/Project"));
        QVERIFY(command.arguments.contains("-S"));
        QVERIFY(command.arguments.contains("-B"));
        QVERIFY(command.arguments.contains("-G"));
        QVERIFY(command.arguments.contains("Ninja"));
        QVERIFY(command.arguments.contains("-DCMAKE_CXX_COMPILER=g++"));
    }

    void createsBuildCommandWithTarget()
    {
        BuildSettings settings;
        settings.cmakeProgram = "cmake";
        settings.buildDirectory = "out/build";

        const auto command = CMakeBuildPlanner::buildCommand("E:/Project", settings, "MalloyWriter");
        QCOMPARE(command.program, QString("cmake"));
        QVERIFY(command.arguments.contains("--build"));
        QVERIFY(command.arguments.contains("--target"));
        QVERIFY(command.arguments.contains("MalloyWriter"));
    }

    void resolvesRelativeBuildDirectory()
    {
        BuildSettings settings;
        settings.buildDirectory = ".malloywriter/build";

        const QString buildDirectory = CMakeBuildPlanner::resolvedBuildDirectory("E:/Project", settings);
        QCOMPARE(buildDirectory, QDir::cleanPath("E:/Project/.malloywriter/build"));
    }
};

QTEST_MAIN(BuildSystemTests)

#include "build_system_tests.moc"

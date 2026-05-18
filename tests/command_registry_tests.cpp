#include "platform/CommandRegistry.hpp"

#include <QtTest/QtTest>

using MalloyWriter::Platform::Command;
using MalloyWriter::Platform::CommandRegistry;

class CommandRegistryTests : public QObject {
    Q_OBJECT

private slots:
    void registersAndExecutesCommands()
    {
        CommandRegistry registry;
        bool executed = false;

        QVERIFY(registry.registerCommand({"file.save", "Save", "File", {}, [&executed]() {
                                              executed = true;
                                          }}));

        QVERIFY(registry.contains("file.save"));
        QVERIFY(registry.execute("file.save"));
        QVERIFY(executed);
    }

    void rejectsDuplicateCommands()
    {
        CommandRegistry registry;
        QVERIFY(registry.registerCommand({"build.build", "Build", "Build", {}, []() {}}));
        QVERIFY(!registry.registerCommand({"build.build", "Build Again", "Build", {}, []() {}}));
        QCOMPARE(registry.commands().size(), 1);
    }

    void refusesMissingCommands()
    {
        CommandRegistry registry;
        QVERIFY(!registry.execute("missing.command"));
        QVERIFY(!registry.contains("missing.command"));
    }
};

QTEST_MAIN(CommandRegistryTests)

#include "command_registry_tests.moc"

#include "platform/WorkspaceModel.hpp"

#include <QSignalSpy>
#include <QTemporaryDir>
#include <QtTest/QtTest>

using MalloyWriter::Platform::WorkspaceModel;

class WorkspaceModelTests : public QObject {
    Q_OBJECT

private slots:
    void opensExistingFolder()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());

        WorkspaceModel workspace;
        QSignalSpy spy(&workspace, &WorkspaceModel::workspaceChanged);

        QVERIFY(workspace.openFolder(dir.path()));
        QVERIFY(workspace.hasWorkspace());
        QCOMPARE(spy.count(), 1);
        QVERIFY(workspace.rootPath().contains(dir.path()));
    }

    void rejectsMissingFolder()
    {
        WorkspaceModel workspace;
        QVERIFY(!workspace.openFolder("Z:/this/folder/should/not/exist"));
        QVERIFY(!workspace.hasWorkspace());
    }
};

QTEST_MAIN(WorkspaceModelTests)

#include "workspace_model_tests.moc"

#include "platform/ProjectService.hpp"

#include <QTemporaryDir>
#include <QtTest/QtTest>

using MalloyWriter::Platform::ProjectService;

class ProjectServiceTests : public QObject {
    Q_OBJECT

private slots:
    void refreshFailsWithoutWorkspace()
    {
        ProjectService service;
        QString error;
        QVERIFY(!service.refreshCMakeModel(&error));
        QVERIFY(!error.isEmpty());
    }

    void openWorkspaceRejectsMissingDirectory()
    {
        ProjectService service;
        QVERIFY(!service.openWorkspace(QStringLiteral("C:/no/such/directory/xyzzy")));
        QVERIFY(!service.hasWorkspace());
    }

    // Regression: refreshCMakeModel's result must not depend on whether an error
    // pointer was supplied (previously it returned success whenever errorMessage
    // was null, masking failures).
    void refreshResultIsIndependentOfErrorPointer()
    {
        ProjectService service;
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        QVERIFY(service.openWorkspace(dir.path()));

        QString error;
        const bool withError = service.refreshCMakeModel(&error);
        const bool withNullptr = service.refreshCMakeModel(nullptr);
        QCOMPARE(withError, withNullptr);
        if (!withError) {
            QVERIFY(!error.isEmpty());
        }
    }
};

QTEST_MAIN(ProjectServiceTests)

#include "project_service_tests.moc"

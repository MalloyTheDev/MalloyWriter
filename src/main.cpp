#include "base/Log.hpp"
#include "workbench/MainWindow.hpp"

#include <QApplication>
#include <QCoreApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("Malloy");
    QCoreApplication::setApplicationName("MalloyWriter");
    QCoreApplication::setApplicationVersion("0.1.0");

    MalloyWriter::Base::installMessageHandler();

    MalloyWriter::Workbench::MainWindow window;
    window.show();

    return app.exec();
}

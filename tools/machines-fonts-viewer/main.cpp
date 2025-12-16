#include <QApplication>

#include "MainWindow.hpp"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    QCoreApplication::setApplicationName("machines-fonts-viewer");
    QCoreApplication::setApplicationVersion("0.1");
    QGuiApplication::setApplicationDisplayName("Machines Fonts Viewer");

    MainWindow window;
    window.show();

    return app.exec();
}

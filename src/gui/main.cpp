#include <QApplication>
#include "MainWindow.hpp"

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    MainWindow w;
    w.show();

    // If a file path is passed as first argument, load it immediately
    if (argc > 1)
    {
        const QString path = QString::fromLocal8Bit(argv[1]);
        w.loadLayoutFromPath(path);
    }
    return app.exec();
}

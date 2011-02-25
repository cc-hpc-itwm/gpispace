#include <QtGui/QApplication>
#include "monitorwindow.hpp"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MonitorWindow w;
    w.show();

    return a.exec();
}

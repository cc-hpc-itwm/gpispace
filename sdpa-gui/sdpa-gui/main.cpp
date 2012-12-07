#include <iostream>

#include <QtCore/QString>
#include <QApplication>
#include "monitorwindow.hpp"

int main(int ac, char *av[])
{
  unsigned short exe_port (0);
  unsigned short log_port (2438);

  if (ac < 2)
  {
    std::cerr << "usage: " << av[0] << " gui-port [log-port]" << std::endl;
    return 1;
  }

  exe_port = QString(av[1]).toUShort ();
  if (ac > 2)
    log_port = QString(av[2]).toUShort ();

  QApplication a(ac, av);
  MonitorWindow w(exe_port, log_port);
  w.show();

  return a.exec();
}

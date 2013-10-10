#include <pnete/ui/execution_monitor.hpp>
#include <pnete/ui/log_monitor.hpp>

#include <fhg/revision.hpp>

#include <QApplication>
#include <QTabWidget>
#include <QtCore/QString>

#include <iostream>

int main (int ac, char *av[])
{
  if (ac < 2)
  {
    std::cerr << "usage: " << av[0] << " gui-port [log-port]" << std::endl;
    return 1;
  }

  QApplication a (ac, av);

  QApplication::setApplicationName ("sdpa-gui");
  QApplication::setApplicationVersion ( QString (fhg::project_version())
                                      .append (" - ")
                                      .append (fhg::project_revision())
                                      );
  QApplication::setOrganizationDomain ("itwm.fraunhofer.de");
  QApplication::setOrganizationName ("Fraunhofer ITWM");

  QTabWidget window;
  window.addTab ( new fhg::pnete::ui::execution_monitor (QString (av[1]).toUShort())
                , QObject::tr ("Execution Monitor")
                );
  window.addTab ( new log_monitor (ac > 2 ? QString (av[2]).toUShort() : 2438)
                , QObject::tr ("Logging")
                );
  window.show();

  return a.exec();
}

#include <pnete/ui/execution_monitor.hpp>
#include <pnete/ui/log_monitor.hpp>

#include <fhg/revision.hpp>
#include <util-generic/print_exception.hpp>

#include <fhg/util/boost/program_options/generic.hpp>
#include <fhg/util/boost/program_options/validators/positive_integral.hpp>

#include <boost/program_options.hpp>

#include <QApplication>
#include <QTabWidget>
#include <QtCore/QString>

#include <iostream>

namespace
{
  namespace option
  {
    namespace po = fhg::util::boost::program_options;

    po::option<po::positive_integral<unsigned short>> const gui_port
      {"gui-port", "gui port"};
    po::option<po::positive_integral<unsigned short>> const log_port
      {"log-port", "log port"};
    po::option<std::vector<fhg::logging::tcp_endpoint>> const emitters
      {"emitters", "list of tcp emitters"};
  }
}

int main (int ac, char *av[])
try
{
  boost::program_options::variables_map const vm
    ( fhg::util::boost::program_options::options ("GPI-Space monitor")
    . require (option::gui_port)
    . require (option::log_port)
    . add (option::emitters)
    . store_and_notify (ac, av)
    );

  QApplication a (ac, av);

  QApplication::setApplicationName ("gspc-monitor");
  QApplication::setApplicationVersion ( QString (fhg::project_version())
                                      .append (" - ")
                                      .append (fhg::project_revision())
                                      );
  QApplication::setOrganizationDomain ("itwm.fraunhofer.de");
  QApplication::setOrganizationName ("Fraunhofer ITWM");

  QTabWidget window;
  window.addTab
    ( new fhg::pnete::ui::execution_monitor
        ( option::gui_port.get_from (vm)
        , option::emitters.get_from_or_value (vm, {})
        )
    , QObject::tr ("Execution Monitor")
    );
  window.addTab ( new log_monitor (option::log_port.get_from (vm))
                , QObject::tr ("Logging")
                );
  window.show();

  return a.exec();
}
catch (...)
{
  std::cerr << "EX: " << fhg::util::current_exception_printer() << '\n';
  return 1;
}

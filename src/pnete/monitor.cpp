#include <pnete/ui/execution_monitor.hpp>
#include <pnete/ui/log_monitor.hpp>

#include <logging/legacy_bridge.hpp>

#include <fhg/revision.hpp>
#include <util-generic/print_exception.hpp>

#include <fhg/util/boost/program_options/generic.hpp>
#include <fhg/util/boost/program_options/validators/positive_integral.hpp>

#include <boost/program_options.hpp>

#include <QApplication>
#include <QTabWidget>
#include <QtCore/QString>

#include <iostream>
#include <vector>

namespace
{
  namespace option
  {
    namespace po = fhg::util::boost::program_options;

    po::option<po::positive_integral<unsigned short>> const log_port
      {"log-port", "log port"};
    po::option<std::vector<fhg::logging::endpoint>> const emitters
      {"emitters", "list of tcp emitters"};
  }
}

int main (int ac, char *av[])
try
{
  boost::program_options::variables_map const vm
    ( fhg::util::boost::program_options::options ("GPI-Space monitor")
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

  fhg::logging::legacy_bridge log_bridge (option::log_port.get_from (vm));

  auto emitters (option::emitters.get_from_or_value (vm, {}));
  emitters.emplace_back (log_bridge.local_endpoint());

  QTabWidget window;
  window.addTab
    ( new fhg::pnete::ui::execution_monitor (emitters)
    , QObject::tr ("Execution Monitor")
    );
  window.addTab
    ( new log_monitor (emitters)
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

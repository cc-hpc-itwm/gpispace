#include <pnete/ui/execution_monitor.hpp>
#include <pnete/ui/log_monitor.hpp>

#include <logging/stream_receiver.hpp>

#include <fhg/revision.hpp>
#include <util-generic/print_exception.hpp>

#include <fhg/util/boost/program_options/generic.hpp>
#include <fhg/util/boost/program_options/validators/positive_integral.hpp>

#include <util-qt/scoped_until_qt_owned_ptr.hpp>

#include <boost/program_options.hpp>

#include <QtCore/QString>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QTabWidget>

#include <iostream>
#include <vector>

namespace
{
  namespace option
  {
    namespace po = fhg::util::boost::program_options;

    po::option<std::vector<fhg::logging::endpoint>> const emitters
      {"emitters", "list of tcp emitters"};
  }
}

int main (int ac, char *av[])
try
{
  boost::program_options::variables_map const vm
    ( fhg::util::boost::program_options::options ("GPI-Space monitor")
    . require (option::emitters)
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

  QMainWindow window;
  fhg::util::qt::scoped_until_qt_owned_ptr<QTabWidget> tabs;
  fhg::util::qt::scoped_until_qt_owned_ptr<log_monitor> logging;
  fhg::util::qt::scoped_until_qt_owned_ptr<fhg::pnete::ui::execution_monitor> gantt;
  tabs->addTab (gantt.release(), QObject::tr ("Execution Monitor"));
  tabs->addTab (logging.release(), QObject::tr ("Logging"));
  window.setCentralWidget (tabs.release());

  fhg::logging::stream_receiver log_receiver
    ( option::emitters.get_from_or_value (vm, {})
    , [&] (fhg::logging::message const& message)
      {
        logging->append_log_event (message);
        gantt->append_event (message);
      }
    );

  auto save_log (window.menuBar()->addAction ("Save Text Log"));
  save_log->setShortcuts (QKeySequence::Save);
  QObject::connect
    (save_log, &QAction::triggered, logging.get(), &log_monitor::save);

  window.show();

  return a.exec();
}
catch (...)
{
  std::cerr << "EX: " << fhg::util::current_exception_printer() << '\n';
  return 1;
}

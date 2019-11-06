#include <pnete/ui/execution_monitor.hpp>
#include <pnete/ui/log_monitor.hpp>

#include <logging/protocol.hpp>
#include <logging/stream_receiver.hpp>

#include <rpc/service_dispatcher.hpp>
#include <rpc/service_handler.hpp>
#include <rpc/service_tcp_provider.hpp>

#include <fhg/revision.hpp>
#include <fhg/util/boost/program_options/generic.hpp>
#include <fhg/util/boost/program_options/validators/positive_integral.hpp>
#include <util-generic/make_optional.hpp>
#include <util-generic/print_exception.hpp>
#include <util-generic/scoped_boost_asio_io_service_with_threads.hpp>
#include <util-generic/this_bound_mem_fn.hpp>
#include <util-generic/wait_and_collect_exceptions.hpp>

#include <util-qt/message_box.hpp>
#include <util-qt/scoped_until_qt_owned_ptr.hpp>

#include <boost/optional.hpp>
#include <boost/program_options.hpp>
#include <boost/utility/in_place_factory.hpp>

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QInputDialog>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QTabWidget>

#include <iostream>
#include <string>
#include <vector>

namespace
{
  namespace option
  {
    namespace po = fhg::util::boost::program_options;

    po::option<std::vector<fhg::logging::endpoint>> const emitters
      {"emitters", "list of tcp emitters"};
    po::option<unsigned short> const port
      {"port", "a port to listen on for new emitters"};
  }

  struct TCPServerProvidingAddEmitters
  {
    fhg::rpc::service_dispatcher service_dispatcher;
    fhg::rpc::service_handler
      <fhg::logging::protocol::receiver::add_emitters> add_emitters;
    fhg::util::scoped_boost_asio_io_service_with_threads io_service = {1};
    fhg::rpc::service_tcp_provider add_emitters_service_provider;

    TCPServerProvidingAddEmitters ( fhg::logging::stream_receiver* log_receiver
                                  , unsigned short port
                                  )
      : add_emitters
          ( service_dispatcher
          , fhg::util::bind_this
              (log_receiver, &fhg::logging::stream_receiver::add_emitters)
          )
      , add_emitters_service_provider
          ( io_service
          , service_dispatcher
          , boost::asio::ip::tcp::endpoint (boost::asio::ip::tcp::v4(), port)
          )
    {}
  };
}

int main (int ac, char *av[])
try
{
  boost::program_options::variables_map const vm
    ( fhg::util::boost::program_options::options ("GPI-Space monitor")
    . add (option::emitters)
    . add (option::port)
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

  boost::optional<TCPServerProvidingAddEmitters>
    tcp_server_providing_add_emitters;
  auto const port (option::port.get<unsigned short> (vm));
  if (port)
  {
    tcp_server_providing_add_emitters = boost::in_place (&log_receiver, *port);
  }

  auto add_emitter (window.menuBar()->addAction ("Add emitters"));
  add_emitter->setShortcuts (QKeySequence::New);
  QObject::connect
    ( add_emitter, &QAction::triggered
    , [&]
      {
        try
        {
          fhg::util::apply_for_each_and_collect_exceptions
            ( QInputDialog::getMultiLineText
                (&window, "Add emitters", "Enter one emitter endpoint per line")
              .split('\n', QString::SkipEmptyParts)
            , [&] (QString line)
              {
                log_receiver.add_emitters
                  ({fhg::logging::endpoint (line.toStdString())});
              }
            );
        }
        catch (...)
        {
          fhg::util::qt::message_box
            ( QMessageBox::Critical
            , &window
            , "Failed to add emitters"
            , QString::fromStdString
                (fhg::util::current_exception_printer().string())
            , fhg::util::qt::button<QMessageBox::Ok> ([]{})
            );
        }
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

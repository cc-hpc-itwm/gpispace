// Copyright (C) 2011,2013-2015,2018-2021,2023-2024,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/monitor/execution_monitor.hpp>
#include <gspc/monitor/log_monitor.hpp>

#include <gspc/logging/stream_receiver.hpp>
#include <gspc/logging/tcp_server_providing_add_emitters.hpp>

#include <gspc/configuration/version.hpp>
#include <gspc/util/boost/program_options/generic.hpp>
#include <gspc/util/boost/program_options/validators/positive_integral.hpp>
#include <gspc/util/print_exception.hpp>
#include <gspc/util/wait_and_collect_exceptions.hpp>

#include <gspc/util/qt/message_box.hpp>
#include <gspc/util/qt/scoped_until_qt_owned_ptr.hpp>

#include <boost/program_options.hpp>

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QInputDialog>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QTabWidget>

#include <functional>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace
{
  namespace option
  {
    namespace po = gspc::util::boost::program_options;

    po::option<std::vector<gspc::logging::endpoint>> const emitters
      {"emitters", "list of tcp emitters"};
    po::option<unsigned short> const port
      {"port", "a port to listen on for new emitters"};
  }
}

int main (int ac, char *av[])
try
{
  ::boost::program_options::variables_map const vm
    ( gspc::util::boost::program_options::options ("GPI-Space monitor")
    . add (option::emitters)
    . add (option::port)
    . store_and_notify (ac, av)
    );

  QApplication a (ac, av);

  QApplication::setApplicationName ("gspc-monitor");
  QApplication::setApplicationVersion (QString (gspc::configuration::version()));
  QApplication::setOrganizationDomain ("itwm.fraunhofer.de");
  QApplication::setOrganizationName ("Fraunhofer ITWM");

  QMainWindow window;
  gspc::util::qt::scoped_until_qt_owned_ptr<QTabWidget> tabs;
  gspc::util::qt::scoped_until_qt_owned_ptr<gspc::monitor::log_monitor> logging;
  gspc::util::qt::scoped_until_qt_owned_ptr<gspc::monitor::execution_monitor> gantt;
  tabs->addTab (gantt.release(), QObject::tr ("Execution Monitor"));
  tabs->addTab (logging.release(), QObject::tr ("Logging"));
  window.setCentralWidget (tabs.release());

  gspc::logging::stream_receiver log_receiver
    ( option::emitters.get_from_or_value (vm, {})
    , [&] (gspc::logging::message const& message)
      {
        logging->append_log_event (message);
        gantt->append_event (message);
      }
    );

  auto const port (option::port.get<unsigned short> (vm));
  auto tcp_server_providing_add_emitters
    { std::invoke
      ( [&]() -> std::optional<gspc::logging::tcp_server_providing_add_emitters>
        {
          if (port)
          {
            return std::optional<gspc::logging::tcp_server_providing_add_emitters>
              {std::in_place, std::addressof (log_receiver), *port};
          }

          return {};
        }
      )
    };

  auto add_emitter (window.menuBar()->addAction ("Add emitters"));
  add_emitter->setShortcuts (QKeySequence::New);
  QObject::connect
    ( add_emitter, &QAction::triggered
    , [&]
      {
        try
        {
          gspc::util::apply_for_each_and_collect_exceptions
            ( QInputDialog::getMultiLineText
                (&window, "Add emitters", "Enter one emitter endpoint per line")
              .split('\n', QString::SkipEmptyParts)
            , [&] (QString line)
              {
                log_receiver.add_emitters_blocking
                  ({gspc::logging::endpoint (line.toStdString())});
              }
            );
        }
        catch (...)
        {
          gspc::util::qt::message_box
            ( QMessageBox::Critical
            , &window
            , "Failed to add emitters"
            , QString::fromStdString
                (gspc::util::current_exception_printer().string())
            , gspc::util::qt::button<QMessageBox::Ok> ([]{})
            );
        }
      }
    );

  auto save_log (window.menuBar()->addAction ("Save Text Log"));
  save_log->setShortcuts (QKeySequence::Save);
  QObject::connect
    (save_log, &QAction::triggered, logging.get(), &gspc::monitor::log_monitor::save);

  window.show();

  return a.exec();
}
catch (...)
{
  std::cerr << "EX: " << gspc::util::current_exception_printer() << '\n';
  return 1;
}

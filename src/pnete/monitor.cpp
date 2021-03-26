// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#include <pnete/ui/execution_monitor.hpp>
#include <pnete/ui/log_monitor.hpp>

#include <logging/stream_receiver.hpp>
#include <logging/tcp_server_providing_add_emitters.hpp>

#include <fhg/project_version.hpp>
#include <fhg/util/boost/program_options/generic.hpp>
#include <fhg/util/boost/program_options/validators/positive_integral.hpp>
#include <util-generic/print_exception.hpp>
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
  QApplication::setApplicationVersion (QString (fhg::project_version()));
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

  boost::optional<fhg::logging::tcp_server_providing_add_emitters>
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
                log_receiver.add_emitters_blocking
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

// This file is part of GPI-Space.
// Copyright (C) 2020 Fraunhofer ITWM
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

#include <pnete/ui/gspc_monitor.hpp>

#include <util-generic/print_exception.hpp>

#include <QApplication>
#include <QString>

#include <iostream>
#include <stdexcept>

int main (int argc, char** argv)
try
{
  QApplication app (argc, argv);

  if (argc != 3)
  {
    std::cerr << "usage: " << argv[0] << " <host> <port>\n";
    return -1;
  }

  const QString host (argv[1]);
  const int port (QString (argv[2]).toInt());

  fhg::pnete::ui::gspc_monitor monitor (host, port);

  monitor.show();

  return app.exec();
}
catch (...)
{
  std::cerr << "error: " << fhg::util::current_exception_printer() << '\n';
  return 1;
}

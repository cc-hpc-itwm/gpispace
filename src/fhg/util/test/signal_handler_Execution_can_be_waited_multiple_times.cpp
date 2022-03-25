// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
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

#include <fhg/util/signal_handler_manager.hpp>

#include <util-generic/print_exception.hpp>
#include <util-generic/latch.hpp>

#include <iostream>
#include <thread>

int main()
try
{
  fhg::util::signal_handler_manager manager;

  fhg::util::Execution execution (manager);

  fhg::util::latch thread_running (1);

  std::thread thread ([&] { thread_running.count_down(); execution.wait(); });

  thread_running.wait();

  execution.stop();

  execution.wait();

  if (thread.joinable()) { thread.join(); }

  return 0;
}
catch (...)
{
  std::cerr << fhg::util::current_exception_printer() << "\n";

  return 1;
}

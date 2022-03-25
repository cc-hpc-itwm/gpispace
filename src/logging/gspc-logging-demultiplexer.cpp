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

#include <logging/demultiplexer.hpp>

#include <rif/started_process_promise.hpp>

#include <util-generic/print_exception.hpp>
#include <util-generic/syscall.hpp>
#include <util-generic/syscall/process_signal_block.hpp>
#include <util-generic/syscall/signal_set.hpp>

#include <cstdlib>
#include <exception>
#include <iostream>

int main (int argc, char** argv)
try
{
  fhg::rif::started_process_promise promise (argc, argv);
  try
  {
    fhg::util::syscall::signal_set const signals {SIGINT, SIGTERM};
    fhg::util::syscall::process_signal_block const signal_block (signals);

    fhg::logging::demultiplexer const actual (promise, argc, argv);

    fhg::util::syscall::sigwaitinfo (&signals._, nullptr);

    return EXIT_SUCCESS;
  }
  catch (...)
  {
    promise.set_exception (std::current_exception());
    return EXIT_FAILURE;
  }
}
catch (...)
{
  std::cerr << fhg::util::current_exception_printer() << "\n";
  return EXIT_FAILURE;
}

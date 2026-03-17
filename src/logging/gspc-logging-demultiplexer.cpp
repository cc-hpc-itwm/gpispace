// Copyright (C) 2019,2023,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/logging/demultiplexer.hpp>

#include <gspc/rif/started_process_promise.hpp>

#include <gspc/util/print_exception.hpp>
#include <gspc/util/syscall.hpp>
#include <gspc/util/syscall/process_signal_block.hpp>
#include <gspc/util/syscall/signal_set.hpp>

#include <cstdlib>
#include <exception>
#include <iostream>

int main (int argc, char** argv)
try
{
  gspc::rif::started_process_promise promise (argc, argv);
  try
  {
    gspc::util::syscall::signal_set const signals {SIGINT, SIGTERM};
    gspc::util::syscall::process_signal_block const signal_block (signals);

    gspc::logging::demultiplexer const actual (promise, argc, argv);

    gspc::util::syscall::sigwaitinfo (&signals._, nullptr);

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
  std::cerr << gspc::util::current_exception_printer() << "\n";
  return EXIT_FAILURE;
}

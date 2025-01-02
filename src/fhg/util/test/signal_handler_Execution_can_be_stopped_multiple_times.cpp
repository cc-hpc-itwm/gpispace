// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <fhg/util/signal_handler_manager.hpp>

#include <util-generic/print_exception.hpp>
#include <util-generic/syscall.hpp>

#include <iostream>
#include <thread>

int main()
try
{
  fhg::util::signal_handler_manager manager;

  fhg::util::Execution execution (manager);

  std::thread thread1 ([&] { execution.stop(); });
  std::thread thread2
    ([&] { fhg::util::syscall::kill (fhg::util::syscall::getpid(), SIGTERM); });

  execution.wait();

  if (thread1.joinable()) { thread1.join(); }
  if (thread2.joinable()) { thread2.join(); }

  return 0;
}
catch (...)
{
  std::cerr << fhg::util::current_exception_printer() << "\n";

  return 1;
}

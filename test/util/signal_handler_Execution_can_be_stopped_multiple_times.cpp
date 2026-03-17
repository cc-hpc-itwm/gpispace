// Copyright (C) 2021,2023,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/util/signal_handler_manager.hpp>

#include <gspc/util/print_exception.hpp>
#include <gspc/util/syscall.hpp>

#include <iostream>
#include <thread>

int main()
try
{
  gspc::util::signal_handler_manager manager;

  gspc::util::Execution execution (manager);

  std::thread thread1 ([&] { execution.stop(); });
  std::thread thread2
    ([&] { gspc::util::syscall::kill (gspc::util::syscall::getpid(), SIGTERM); });

  execution.wait();

  if (thread1.joinable()) { thread1.join(); }
  if (thread2.joinable()) { thread2.join(); }

  return 0;
}
catch (...)
{
  std::cerr << gspc::util::current_exception_printer() << "\n";

  return 1;
}

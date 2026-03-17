// Copyright (C) 2021-2023,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/util/signal_handler_manager.hpp>

#include <gspc/util/print_exception.hpp>

#include <future>
#include <iostream>
#include <thread>

int main()
try
{
  gspc::util::signal_handler_manager manager;

  gspc::util::Execution execution (manager);

  std::promise<void> thread_running;

  std::thread thread ([&] { thread_running.set_value(); execution.wait(); });

  thread_running.get_future().wait();

  execution.stop();

  execution.wait();

  if (thread.joinable()) { thread.join(); }

  return 0;
}
catch (...)
{
  std::cerr << gspc::util::current_exception_printer() << "\n";

  return 1;
}

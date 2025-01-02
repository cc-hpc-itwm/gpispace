// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <fhg/util/signal_handler_manager.hpp>

#include <util-generic/print_exception.hpp>

#include <iostream>
#include <thread>

#define TEST(STOP_METHOD...)                                            \
  int main()                                                            \
  try                                                                   \
  {                                                                     \
    fhg::util::signal_handler_manager manager;                          \
                                                                        \
    fhg::util::Execution execution (manager);                           \
                                                                        \
    std::thread thread ([&] { STOP_METHOD; });                          \
                                                                        \
    execution.wait();                                                   \
                                                                        \
    if (thread.joinable()) { thread.join(); }                           \
                                                                        \
    return 0;                                                           \
  }                                                                     \
  catch (...)                                                           \
  {                                                                     \
    std::cerr << fhg::util::current_exception_printer() << "\n";        \
                                                                        \
    return 1;                                                           \
  }

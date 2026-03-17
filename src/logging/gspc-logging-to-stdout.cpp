// Copyright (C) 2019-2021,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/logging/endpoint.hpp>
#include <gspc/logging/stdout_sink.hpp>
#include <gspc/logging/tcp_server_providing_add_emitters.hpp>

#include <gspc/util/boost/program_options/generic.hpp>
#include <gspc/util/print_exception.hpp>
#include <gspc/util/syscall.hpp>
#include <gspc/util/syscall/process_signal_block.hpp>
#include <gspc/util/syscall/signal_set.hpp>

#include <optional>

#include <iostream>
#include <vector>

namespace option
{
  namespace
  {
    namespace po = gspc::util::boost::program_options;
    po::option<std::vector<gspc::logging::endpoint>> const emitters
      {"emitters", "list of emitters"};
    po::option<unsigned short> const port
      {"port", "a port to listen on for new emitters"};
  }
}

int main (int argc, char** argv)
try
{
  gspc::util::syscall::signal_set const signals {SIGINT, SIGTERM};
  gspc::util::syscall::process_signal_block const signal_block (signals);

  ::boost::program_options::variables_map const vm
    ( gspc::util::boost::program_options::options ("GPI-Space Logging to stdout")
    . add (option::emitters)
    . add (option::port)
    . store_and_notify (argc, argv)
    );

  gspc::logging::stdout_sink impl (option::emitters.get_from_or_value (vm, {}));

  std::optional<gspc::logging::tcp_server_providing_add_emitters>
    tcp_server_providing_add_emitters;
  auto const port (option::port.get<unsigned short> (vm));
  if (port)
  {
    tcp_server_providing_add_emitters.emplace (&impl, *port);
  }

  gspc::util::syscall::sigwaitinfo (&signals._, nullptr);

  return EXIT_SUCCESS;
}
catch (...)
{
  std::cerr << gspc::util::current_exception_printer() << "\n";
  return EXIT_FAILURE;
}

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

#include <logging/endpoint.hpp>
#include <logging/stdout_sink.hpp>
#include <logging/tcp_server_providing_add_emitters.hpp>

#include <util-generic/boost/program_options/generic.hpp>
#include <util-generic/print_exception.hpp>
#include <util-generic/syscall.hpp>
#include <util-generic/syscall/process_signal_block.hpp>
#include <util-generic/syscall/signal_set.hpp>

#include <boost/utility/in_place_factory.hpp>

#include <iostream>
#include <vector>

namespace option
{
  namespace
  {
    namespace po = fhg::util::boost::program_options;
    po::option<std::vector<fhg::logging::endpoint>> const emitters
      {"emitters", "list of emitters"};
    po::option<unsigned short> const port
      {"port", "a port to listen on for new emitters"};
  }
}

int main (int argc, char** argv)
try
{
  fhg::util::syscall::signal_set const signals {SIGINT, SIGTERM};
  fhg::util::syscall::process_signal_block const signal_block (signals);

  ::boost::program_options::variables_map const vm
    ( fhg::util::boost::program_options::options ("GPI-Space Logging to stdout")
    . add (option::emitters)
    . add (option::port)
    . store_and_notify (argc, argv)
    );

  fhg::logging::stdout_sink impl (option::emitters.get_from_or_value (vm, {}));

  ::boost::optional<fhg::logging::tcp_server_providing_add_emitters>
    tcp_server_providing_add_emitters;
  auto const port (option::port.get<unsigned short> (vm));
  if (port)
  {
    tcp_server_providing_add_emitters = ::boost::in_place (&impl, *port);
  }

  fhg::util::syscall::sigwaitinfo (&signals._, nullptr);

  return EXIT_SUCCESS;
}
catch (...)
{
  std::cerr << fhg::util::current_exception_printer() << "\n";
  return EXIT_FAILURE;
}

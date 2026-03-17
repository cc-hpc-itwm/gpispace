// Copyright (C) 2020-2021,2023-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <bin/run.hpp>
#include <util/print_exception.hpp>

#include "pi.hpp"
#include <exception>
#include <ios>
#include <iostream>
#include <ostream>
#include <tuple>
#include <utility>

int main (int argc, char** argv)
try
{
  stochastic_with_heureka::workflow_result const workflow_result
    ( stochastic_with_heureka::run
      ( argc
      , argv
      , std::nullopt
      , "pi"
      , [] (::boost::program_options::variables_map const&)
        {
          return gspc::we::type::bytearray();
        }
      )
    );

  std::cout << "got_heureka = "
            << std::boolalpha << workflow_result.got_heureka() << std::endl;
  std::cout << "number_of_rolls_done = "
            << workflow_result.number_of_rolls_done() << std::endl;

  using gspc::share::example::stochastic_with_heurake::pi::Result;

  Result result;
  workflow_result.result().copy (&result);

  std::cout << "result = "
            << result.result.in
            << " / "
            << result.result.n << std::endl;
  std::cout << "reduced = "
            << result.reduced.in
            << " / "
            << result.reduced.n << std::endl;
  std::cout << "PI = " << result.pi << std::endl;
  std::cout << "err = " << ( result.pi
                           - 3.1415926535897932384626433832795028841968
                           ) << std::endl;

  return 0;
}
catch (std::exception const& e)
{
  std::cout << "EXCEPTION: " << util::print_exception (e) << std::endl;

  return -1;
}

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

#include <bin/run.hpp>
#include <util/print_exception.hpp>

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
      , boost::none
      , "pi"
      , [] (boost::program_options::variables_map const&)
        {
          return we::type::bytearray();
        }
      )
    );

  std::cout << "got_heureka = "
            << std::boolalpha << workflow_result.got_heureka() << std::endl;
  std::cout << "number_of_rolls_done = "
            << workflow_result.number_of_rolls_done() << std::endl;

  std::tuple< std::pair<unsigned long, unsigned long>
            , std::pair<unsigned long, unsigned long>
            , double
            > result;
  workflow_result.result().copy (&result);

  std::cout << "result = "
            << std::get<0> (result).first
            << " / "
            << std::get<0> (result).second << std::endl;
  std::cout << "reduced = "
            << std::get<1> (result).first
            << " / "
            << std::get<1> (result).second << std::endl;
  std::cout << "PI = " << std::get<2> (result) << std::endl;
  std::cout << "err = " << ( std::get<2> (result)
                           - 3.1415926535897932384626433832795028841968
                           ) << std::endl;

  return 0;
}
catch (std::exception const& e)
{
  std::cout << "EXCEPTION: " << util::print_exception (e) << std::endl;

  return -1;
}

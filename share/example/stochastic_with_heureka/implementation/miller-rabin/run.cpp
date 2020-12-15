// This file is part of GPI-Space.
// Copyright (C) 2020 Fraunhofer ITWM
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

//! \note workaround for gmp bug: cstddef included with libstdcxx
//! implementation detail that changed in gcc 4.9.0
#include <cstddef>
#include <gmpxx.h>

#include <iostream>
#include <iomanip>

namespace
{
  namespace option
  {
    constexpr char const* const to_test {"to-test"};
  }

  boost::program_options::options_description options()
  {
    boost::program_options::options_description description;

    //! \todo validators
    description.add_options()
      ( option::to_test
      , boost::program_options::value<std::string>()->required()
      , "Number to test"
      );

    return description;
  }
}

int main (int argc, char** argv)
try
{
  stochastic_with_heureka::workflow_result const workflow_result
    ( stochastic_with_heureka::run
      ( argc
      , argv
      , options()
      , "miller-rabin"
      , [] (boost::program_options::variables_map const& vm)
        {
          size_t count;

          void* buffer
            ( mpz_export
              ( nullptr, &count
              , 1, sizeof (char), 0, 0
              , mpz_class (vm[option::to_test].as<std::string>()).get_mpz_t()
              )
            );

          return we::type::bytearray
            (std::string (static_cast<char*> (buffer), count));
        }
      )
    );

  std::cout << "got_heureka = "
            << std::boolalpha << workflow_result.got_heureka() << std::endl;
  std::cout << "number_of_rolls_done = "
            << workflow_result.number_of_rolls_done() << std::endl;

  bool result;
  workflow_result.result().copy (&result);

  std::cout << "result = " << (result ? "composite" : "probably prime")
            << std::endl;

  return 0;
}
catch (std::exception const& e)
{
  std::cout << "EXCEPTION: " << e.what() << std::endl;

  return -1;
}

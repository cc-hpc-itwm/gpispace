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

#include <miller-rabin/util.hpp>

#include <util/print_exception.hpp>

#include <we/type/bytearray.hpp>
#include <we/type/value/read.hpp>

#include <iostream>
#include <ostream>
#include <string>

int main (int argc, char** argv)
try
{
  if (argc != 2)
  {
    std::cerr << "usage: " << argv[0] << " <serialized_result>" << std::endl;
    return 1;
  }

  miller_rabin::show_result
    ( std::cout
    , ::boost::get<we::type::bytearray>
        (pnet::type::value::read (std::string (argv[1])))
    );

  return 0;
}
catch (std::exception const& e)
{
  std::cout << "EXCEPTION: " << util::print_exception (e) << std::endl;

  return -1;
}

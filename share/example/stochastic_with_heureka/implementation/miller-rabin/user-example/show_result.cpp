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

#include <we/type/bytearray.hpp>
#include <we/type/value/read.hpp>

#include <iostream>

int main (int argc, char** argv)
{
  if (argc != 2)
  {
    std::cerr << "usage: " << argv[0] << " <serialized_result>" << std::endl;
    return 1;
  }

  bool result;
  boost::get<we::type::bytearray>
    (pnet::type::value::read (std::string (argv[1]))).copy (&result);

  std::cout << "result = " << (result ? "composite" : "probably prime")
            << std::endl;
  return 0;
}

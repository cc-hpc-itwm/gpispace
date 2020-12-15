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

#include <miller-rabin/util.hpp>

#include <we/type/value/show.hpp>

#include <iostream>

namespace
{
  we::type::bytearray to_bytearray (std::string number)
  {
    size_t count;
    void* buffer
      ( mpz_export
        (NULL, &count, 1, sizeof (char), 0, 0, mpz_class (number).get_mpz_t())
      );
    return we::type::bytearray
      (std::string (static_cast<char*> (buffer), count));
  }
}

int main (int argc, char** argv)
{
  if (argc != 2)
  {
    std::cerr << "usage: " << argv[0] << " <number>" << std::endl;
    return 1;
  }

  std::cout << pnet::type::value::show
                 (pnet::type::value::value_type (to_bytearray (argv[1])))
            << std::endl;

  return 0;
}

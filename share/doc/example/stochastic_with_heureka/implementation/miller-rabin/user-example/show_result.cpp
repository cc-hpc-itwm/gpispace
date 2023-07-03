// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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

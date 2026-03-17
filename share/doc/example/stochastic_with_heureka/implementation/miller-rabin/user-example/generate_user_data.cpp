// Copyright (C) 2020-2021,2023,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <miller-rabin/util.hpp>

#include <util/print_exception.hpp>

#include <gspc/we/type/value/show.hpp>

#include <cstddef>
#include <iostream>
#include <ostream>
#include <string>

int main (int argc, char** argv)
try
{
  if (argc != 2)
  {
    std::cerr << "usage: " << argv[0] << " <number>" << std::endl;
    return 1;
  }

  std::cout << gspc::pnet::type::value::show
                 ( gspc::pnet::type::value::value_type
                     (miller_rabin::generate_user_data (argv[1]))
                 )
            << std::endl;

  return 0;
}
catch (std::exception const& e)
{
  std::cout << "EXCEPTION: " << util::print_exception (e) << std::endl;

  return -1;
}

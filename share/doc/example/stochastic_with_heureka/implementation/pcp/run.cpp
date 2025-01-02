// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <bin/run.hpp>
#include <util/print_exception.hpp>

#include <implementation/pcp/pcp.hpp>

#include <exception>
#include <ios>
#include <iostream>
#include <list>
#include <ostream>
#include <string>
#include <utility>

int main (int argc, char** argv)
try
{
  stochastic_with_heureka::workflow_result const workflow_result
    ( stochastic_with_heureka::run
      ( argc
      , argv
      , std::nullopt
      , "pcp"
      , [] (::boost::program_options::variables_map const&)
        {
          std::list<std::pair<std::string, std::string>> rules;
          rules.push_back ({std::string ("abb"), std::string ("a")});
          rules.push_back ({std::string ("bb"), std::string ("bbb")});
          rules.push_back ({std::string ("cbbb"), std::string ("c")});
          // rules.push_back ({std::string ("0"), std::string ("001")});
          // rules.push_back ({std::string ("001"), std::string ("1")});
          // rules.push_back ({std::string ("1"), std::string ("0")});

          return pcp::pcp (rules).bytearray();
        }
      )
    );

  std::cout << "got_heureka = "
            << std::boolalpha << workflow_result.got_heureka() << std::endl;
  std::cout << "number_of_rolls_done = "
            << workflow_result.number_of_rolls_done() << std::endl;

  if (workflow_result.got_heureka())
  {
    std::string const path (workflow_result.result().to_string());

    std::cout << "solution " << path << std::endl;
  }

  return 0;
}
catch (std::exception const& e)
{
  std::cout << "EXCEPTION: " << util::print_exception (e) << std::endl;

  return -1;
}
